//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file accel/detail/HitProcessor.cc
//---------------------------------------------------------------------------//
#include "HitProcessor.hh"

#include <string>
#include <utility>
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4LogicalVolume.hh>
#include <G4Step.hh>
#include <G4StepPoint.hh>
#include <G4ThreeVector.hh>
#include <G4TouchableHandle.hh>
#include <G4TouchableHistory.hh>
#include <G4Track.hh>
#include <G4TransportationManager.hh>
#include <G4VSensitiveDetector.hh>
#include <G4Version.hh>

#include "corecel/cont/EnumArray.hh"
#include "corecel/cont/Range.hh"
#include "corecel/io/Logger.hh"
#include "corecel/sys/ScopedProfiling.hh"
#include "corecel/sys/TraceCounter.hh"
#include "geocel/g4/Convert.hh"
#include "celeritas/Quantities.hh"
#include "celeritas/Types.hh"
#include "celeritas/ext/GeantUnits.hh"
#include "celeritas/user/DetectorSteps.hh"
#include "celeritas/user/StepData.hh"

#include "LevelTouchableUpdater.hh"
#include "NaviTouchableUpdater.hh"

namespace celeritas
{
namespace detail
{
namespace
{
//---------------------------------------------------------------------------//
/*!
 * Get the geometry step status when volume instance IDs are present.
 *
 * Note that this isn't entirely accurate if crossing from one
 * replica/parameterised region to another. For that we will need to map
 * distinct (instance id, replica number) to unique volume instances. Perhaps
 * that would be better done by using "touchables" globally and reconstructing
 * volume instances in post.
 */
G4StepStatus
get_step_status(DetectorStepOutput const& out, size_type step_index)
{
    auto pre = LevelTouchableUpdater::volume_instances(
        out, step_index, StepPoint::pre);
    auto post = LevelTouchableUpdater::volume_instances(
        out, step_index, StepPoint::post);
    for (auto i : range(out.volume_instance_depth))
    {
        if (pre[i] != post[i])
        {
            // Volume instance changed
            if (i == 0 && post[i] == VolumeInstanceId{})
            {
                // Exited the geometry
                return G4StepStatus::fWorldBoundary;
            }
            // Changed volumes
            return G4StepStatus::fGeomBoundary;
        }
        if (!pre[i])
        {
            // Empty volume sentinel encountered: exit
            break;
        }
    }
    return G4StepStatus::fUserDefinedLimit;
}

//---------------------------------------------------------------------------//
}  // namespace

//---------------------------------------------------------------------------//
/*!
 * Construct local navigator and step data.
 */
HitProcessor::HitProcessor(SPConstVecLV detector_volumes,
                           SPConstGeo const& geo,
                           VecParticle const& particles,
                           StepSelection const& selection,
                           StepPointBool const& locate_touchable)
    : detector_volumes_(std::move(detector_volumes))
    , step_post_status_{
          selection.points[StepPoint::pre].volume_instance_ids
          && selection.points[StepPoint::post].volume_instance_ids}
{
    CELER_EXPECT(detector_volumes_ && !detector_volumes_->empty());
    CELER_EXPECT(geo);

    CELER_LOG_LOCAL(debug) << "Setting up hit processor for "
                           << detector_volumes_->size()
                           << " sensitive detectors";

    // Create step and step-owned structures
    step_ = std::make_unique<G4Step>();
    step_->NewSecondaryVector();

#if G4VERSION_NUMBER >= 1103
#    define HP_CLEAR_STEP_POINT(CMD) step_->CMD(nullptr)
#else
#    define HP_CLEAR_STEP_POINT(CMD) /* no "reset" before v11.0.3 */
#endif

#define HP_SETUP_POINT(LOWER, TITLE)                      \
    do                                                    \
    {                                                     \
        if (!selection.points[StepPoint::LOWER])          \
        {                                                 \
            HP_CLEAR_STEP_POINT(Reset##TITLE##StepPoint); \
        }                                                 \
        else                                              \
        {                                                 \
            auto* sp = step_->Get##TITLE##StepPoint();    \
            sp->SetStepStatus(fUserDefinedLimit);         \
            step_points_[StepPoint::LOWER] = sp;          \
        }                                                 \
    } while (0)

    HP_SETUP_POINT(pre, Pre);
    HP_SETUP_POINT(post, Post);
#undef HP_SETUP_POINT
#undef HP_CLEAR_STEP_POINT

    for (auto p : range(StepPoint::size_))
    {
        if (locate_touchable[p])
        {
            // Create touchable handle for this step point
            touch_handle_[p] = new G4TouchableHistory;
            CELER_ASSERT(step_points_[p]);
            step_points_[p]->SetTouchableHandle(touch_handle_[p]);
        }
        if (locate_touchable[p] && !update_touchable_)
        {
            // Create touchable updater
            if constexpr (CELERITAS_CORE_GEO == CELERITAS_CORE_GEO_ORANGE)
            {
                // ORANGE doesn't yet support level reconstruction: see
                // GeantSd.cc
                CELER_EXPECT(selection.points[p].pos
                             && selection.points[p].dir);
                update_touchable_ = std::make_unique<NaviTouchableUpdater>(
                    detector_volumes_);
            }
            else
            {
                CELER_EXPECT(selection.points[p].volume_instance_ids);
                update_touchable_
                    = std::make_unique<LevelTouchableUpdater>(geo);
            }
        }
    }

    // Set invalid values for unsupported SD attributes
    step_->SetNonIonizingEnergyDeposit(
        -std::numeric_limits<double>::infinity());
    for (G4StepPoint* p : step_points_)
    {
        if (!p)
        {
            continue;
        }
        // Time since track was created
        p->SetLocalTime(std::numeric_limits<double>::infinity());
        // Time in rest frame since track was created
        p->SetProperTime(std::numeric_limits<double>::infinity());
        // Speed
        p->SetVelocity(std::numeric_limits<double>::infinity());
        // Safety distance
        p->SetSafety(std::numeric_limits<double>::infinity());
        // Polarization (default to zero)
        p->SetPolarization(G4ThreeVector());
    }

    // Create track if user requested particle types
    for (G4ParticleDefinition const* pd : particles)
    {
        CELER_ASSERT(pd);
        tracks_.emplace_back(new G4Track(
            new G4DynamicParticle(pd, G4ThreeVector()), 0.0, G4ThreeVector()));
        tracks_.back()->SetTrackID(-1);
        tracks_.back()->SetParentID(-1);
    }

    // Convert logical volumes (global) to sensitive detectors (thread local)
    detectors_.resize(detector_volumes_->size());
    for (auto i : range(detectors_.size()))
    {
        G4LogicalVolume const* lv = (*detector_volumes_)[i];
        CELER_ASSERT(lv);
        detectors_[i] = lv->GetSensitiveDetector();
        CELER_VALIDATE(detectors_[i],
                       << "no sensitive detector is attached to volume '"
                       << lv->GetName() << "'@"
                       << static_cast<void const*>(lv));
    }

    CELER_ENSURE(!detectors_.empty());
}

//---------------------------------------------------------------------------//
//! Log on destruction
HitProcessor::~HitProcessor()
{
    try
    {
        CELER_LOG_LOCAL(debug) << "Deallocating hit processor";
    }
    catch (...)  // NOLINT(bugprone-empty-catch)
    {
        // Ignore anything bad that happens while logging
    }
}

//---------------------------------------------------------------------------//
/*!
 * Process detector tallies (CPU).
 */
void HitProcessor::operator()(StepStateHostRef const& states)
{
    copy_steps(&steps_, states);
    if (steps_)
    {
        (*this)(steps_);
    }
}

//---------------------------------------------------------------------------//
/*!
 * Process detector tallies (GPU).
 */
void HitProcessor::operator()(StepStateDeviceRef const& states)
{
    copy_steps(&steps_, states);
    if (steps_)
    {
        (*this)(steps_);
    }
}

//---------------------------------------------------------------------------//
/*!
 * Generate and call hits from a detector output.
 *
 * In an application setting, this is always called with our local data \c
 * steps_ as an argument. For tests, we can call this function explicitly using
 * local test data.
 */
void HitProcessor::operator()(DetectorStepOutput const& out) const
{
    ScopedProfiling profile_this{"process-hits"};
    trace_counter("process-hits", out.size());

    CELER_LOG_LOCAL(debug) << "Processing " << out.size() << " hits";

    for (auto i : range(out.size()))
    {
        (*this)(out, i);
    }
}

//---------------------------------------------------------------------------//
/*!
 * Generate and call a single hit.
 */
void HitProcessor::operator()(DetectorStepOutput const& out, size_type i) const
{
    CELER_EXPECT(!out.detector.empty());
    CELER_EXPECT(i < out.size());
#define HP_SET(SETTER, OUT, UNITS)                   \
    do                                               \
    {                                                \
        if (!OUT.empty())                            \
        {                                            \
            SETTER(convert_to_geant(OUT[i], UNITS)); \
        }                                            \
    } while (0)

    G4LogicalVolume const* lv = this->detector_volume(out.detector[i]);

    HP_SET(step_->SetTotalEnergyDeposit, out.energy_deposition, CLHEP::MeV);
    HP_SET(step_->SetStepLength, out.step_length, clhep_length);

    for (auto sp : range(StepPoint::size_))
    {
        auto* g4sp = step_points_[sp];
        if (!g4sp)
        {
            continue;
        }

        if (auto& touch_handle = touch_handle_[sp])
        {
            CELER_ASSERT(update_touchable_);
            // Update navigation state
            bool success = (*update_touchable_)(out, i, sp, touch_handle());

            if (CELER_UNLIKELY(!success))
            {
                // Inconsistent touchable: skip this energy deposition
                CELER_LOG_LOCAL(error)
                    << "Omitting energy deposition of "
                    << step_->GetTotalEnergyDeposit() / CLHEP::MeV << " [MeV]";
                return;
            }
        }

        HP_SET(g4sp->SetGlobalTime, out.points[sp].time, clhep_time);
        HP_SET(g4sp->SetPosition, out.points[sp].pos, clhep_length);
        HP_SET(g4sp->SetKineticEnergy, out.points[sp].energy, CLHEP::MeV);
        HP_SET(g4sp->SetMomentumDirection, out.points[sp].dir, 1);
        /*!
         * \todo Celeritas currently ignores incoming particle weight and
         * does not perform any variance reduction. See issue #1268.
         */
        g4sp->SetWeight(1.0);

        G4LogicalVolume const* point_lv = [&]() -> G4LogicalVolume const* {
            if (sp == StepPoint::pre)
                return lv;

            // NOTE: post-step volume is only fetched if we're locating the
            // touchable
            if (auto* touch = g4sp->GetTouchable())
            {
                // The physical volume could be null if post-step is outside
                if (auto* pv = touch->GetVolume())
                {
                    return pv->GetLogicalVolume();
                }
            }
            return nullptr;
        }();

        if (point_lv)
        {
            // Copy attributes from logical volume
            g4sp->SetMaterial(point_lv->GetMaterial());
            g4sp->SetMaterialCutsCouple(point_lv->GetMaterialCutsCouple());
            g4sp->SetSensitiveDetector(point_lv->GetSensitiveDetector());
        }
    }
#undef HP_SET

    if (!tracks_.empty())
    {
        // Set the track particle type
        CELER_ASSERT(!out.particle.empty());
        this->update_track(out.particle[i]);
    }

    if (step_post_status_)
    {
        // Update the post-step status based on the geometry instances
        auto* g4sp = step_->GetPostStepPoint();
        CELER_ASSERT(g4sp);
        g4sp->SetStepStatus(get_step_status(out, i));
    }

    // Hit sensitive detector
    this->detector(out.detector[i])->Hit(step_.get());
}

//---------------------------------------------------------------------------//
/*!
 * Recreate the track from the particle ID and saved post-step data.
 *
 * This is a bit like \c G4Step::UpdateTrack .
 */
void HitProcessor::update_track(ParticleId id) const
{
    CELER_EXPECT(id < tracks_.size());
    G4Track& track = *tracks_[id.unchecked_get()];
    step_->SetTrack(&track);

    G4ParticleDefinition const& pd = *track.GetParticleDefinition();

    for (G4StepPoint* p : step_points_)
    {
        if (!p)
        {
            continue;
        }

        // Copy data from track to step points
        p->SetMass(pd.GetPDGMass());
        p->SetCharge(pd.GetPDGCharge());
    }
    if (G4StepPoint* post_step = step_points_[StepPoint::post])
    {
        // Copy data from post-step to track
        track.SetGlobalTime(post_step->GetGlobalTime());
        track.SetPosition(post_step->GetPosition());
        track.SetKineticEnergy(post_step->GetKineticEnergy());
        track.SetMomentumDirection(post_step->GetMomentumDirection());
        track.SetWeight(post_step->GetWeight());
    }
}

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
