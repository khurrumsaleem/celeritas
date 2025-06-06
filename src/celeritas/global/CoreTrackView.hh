//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/global/CoreTrackView.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/random/engine/RngEngine.hh"
#include "corecel/sys/ThreadId.hh"
#include "celeritas/geo/GeoMaterialView.hh"
#include "celeritas/geo/GeoTrackView.hh"
#include "celeritas/mat/MaterialTrackView.hh"
#include "celeritas/phys/CutoffView.hh"
#include "celeritas/phys/ParticleTrackView.hh"
#include "celeritas/phys/PhysicsStepView.hh"
#include "celeritas/phys/PhysicsTrackView.hh"
#include "celeritas/track/SimTrackView.hh"

#include "CoreTrackData.hh"

#if !CELER_DEVICE_COMPILE
#    include "corecel/io/Logger.hh"
#endif

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Helper class to create views from core track data.
 *
 * TODO: const correctness? (Maybe have to wait till C++23's "deducing this"?)
 */
class CoreTrackView
{
  public:
    //!@{
    //! \name Type aliases
    using ParamsRef = NativeCRef<CoreParamsData>;
    using StateRef = NativeRef<CoreStateData>;
    //!@}

  public:
    // Construct with comprehensive param/state data and thread
    inline CELER_FUNCTION CoreTrackView(ParamsRef const& params,
                                        StateRef const& states,
                                        ThreadId thread);

    // Construct directly from a track slot ID
    inline CELER_FUNCTION CoreTrackView(ParamsRef const& params,
                                        StateRef const& states,
                                        TrackSlotId slot);

    // Initialize the track states
    inline CELER_FUNCTION CoreTrackView& operator=(TrackInitializer const&);

    // Return a simulation management view
    inline CELER_FUNCTION SimTrackView sim() const;

    // Return a geometry view
    inline CELER_FUNCTION GeoTrackView geometry() const;

    // Return a geometry-material view
    inline CELER_FUNCTION GeoMaterialView geo_material() const;

    // Return a material view
    inline CELER_FUNCTION MaterialTrackView material() const;

    // Return a particle view
    inline CELER_FUNCTION ParticleTrackView particle() const;

    // Return a particle view of another particle type
    inline CELER_FUNCTION ParticleView particle_record(ParticleId) const;

    // Return a cutoff view
    inline CELER_FUNCTION CutoffView cutoff() const;

    // Return a physics view
    inline CELER_FUNCTION PhysicsTrackView physics() const;

    // Return a view to temporary physics data
    inline CELER_FUNCTION PhysicsStepView physics_step() const;

    // Return an RNG engine
    inline CELER_FUNCTION RngEngine rng() const;

    // Get the index of the current thread in the current kernel
    inline CELER_FUNCTION ThreadId thread_id() const;

    // Get the track's index among the states
    inline CELER_FUNCTION TrackSlotId track_slot_id() const;

    // Action ID for encountering a geometry boundary
    inline CELER_FUNCTION ActionId boundary_action() const;

    // Action ID for some other propagation limit (e.g. field stepping)
    inline CELER_FUNCTION ActionId propagation_limit_action() const;

    // Action ID for being abandoned while looping
    inline CELER_FUNCTION ActionId tracking_cut_action() const;

    // HACK: return scalars (maybe have a struct for all actions?)
    inline CELER_FUNCTION CoreScalars const& core_scalars() const;

    // clang-format off
    // DEPRECATED: remove in v0.7
    [[deprecated]] CELER_FUNCTION SimTrackView make_sim_view() const { return this->sim(); }
    [[deprecated]] CELER_FUNCTION GeoTrackView make_geo_view() const { return this->geometry(); }
    [[deprecated]] CELER_FUNCTION GeoMaterialView make_geo_material_view() const { return this->geo_material(); }
    [[deprecated]] CELER_FUNCTION MaterialTrackView make_material_view() const { return this->material(); }
    [[deprecated]] CELER_FUNCTION ParticleTrackView make_particle_view() const { return this->particle(); }
    [[deprecated]] CELER_FUNCTION ParticleView make_particle_view(ParticleId pid) const { return this->particle_record(pid); }
    [[deprecated]] CELER_FUNCTION CutoffView make_cutoff_view() const { return this->cutoff(); }
    [[deprecated]] CELER_FUNCTION PhysicsTrackView make_physics_view() const { return this->physics(); }
    [[deprecated]] CELER_FUNCTION PhysicsStepView make_physics_step_view() const { return this->physics_step(); }
    [[deprecated]] CELER_FUNCTION RngEngine make_rng_engine() const { return this->rng(); }
    // clang-format on

    //// MUTATORS ////

    // Set the 'errored' flag and tracking cut post-step action
    inline CELER_FUNCTION void apply_errored();

  private:
    StateRef const& states_;
    ParamsRef const& params_;
    ThreadId const thread_id_;
    TrackSlotId track_slot_id_;
};

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Construct with comprehensive param/state data and thread.
 */
CELER_FUNCTION
CoreTrackView::CoreTrackView(ParamsRef const& params,
                             StateRef const& states,
                             ThreadId thread)
    : states_(states), params_(params), thread_id_(thread)
{
    CELER_EXPECT(states_.track_slots.empty()
                 || thread_id_ < states_.track_slots.size());
    track_slot_id_ = TrackSlotId{states_.track_slots.empty()
                                     ? thread_id_.get()
                                     : states_.track_slots[thread_id_]};
    CELER_ENSURE(track_slot_id_ < states_.size());
}

//---------------------------------------------------------------------------//
/*!
 * Construct with comprehensive param/state data and track slot.
 *
 * This signature is used for creating a view of a \em second track in a kernel
 * for initialization.
 */
CELER_FUNCTION
CoreTrackView::CoreTrackView(ParamsRef const& params,
                             StateRef const& states,
                             TrackSlotId track_slot)
    : states_(states), params_(params), track_slot_id_(track_slot)
{
    CELER_EXPECT(track_slot_id_ < states_.size());
}

//---------------------------------------------------------------------------//
/*!
 * Initialize the track states.
 */
CELER_FUNCTION CoreTrackView&
CoreTrackView::operator=(TrackInitializer const& init)
{
    CELER_EXPECT(init);

    // Initialize the simulation state
    this->sim() = init.sim;

    // Initialize the particle attributes
    this->particle() = init.particle;

    // Initialize the geometry
    auto geo = this->geometry();
    geo = init.geo;
    if (CELER_UNLIKELY(geo.failed() || geo.is_outside()))
    {
#if !CELER_DEVICE_COMPILE
        if (!geo.failed())
        {
            // Print an error message if initialization was "successful" but
            // track is outside
            CELER_LOG_LOCAL(error) << R"(Track started outside the geometry)";
        }
        else
        {
            // Do not print anything: the geometry track view itself should've
            // printed a detailed error message
        }
#endif
        this->apply_errored();
        return *this;
    }

    // Initialize the material
    auto matid = this->geo_material().material_id(geo.volume_id());
    if (CELER_UNLIKELY(!matid))
    {
#if !CELER_DEVICE_COMPILE
        CELER_LOG_LOCAL(error) << "Track started in an unknown material";
#endif
        this->apply_errored();
        return *this;
    }
    this->material() = {matid};

    // Initialize the physics state
    this->physics() = {};

    return *this;
}

//---------------------------------------------------------------------------//
/*!
 * Return a simulation management view.
 */
CELER_FUNCTION SimTrackView CoreTrackView::sim() const
{
    return SimTrackView{params_.sim, states_.sim, this->track_slot_id()};
}

//---------------------------------------------------------------------------//
/*!
 * Return a geometry view.
 */
CELER_FUNCTION auto CoreTrackView::geometry() const -> GeoTrackView
{
    return GeoTrackView{
        params_.geometry, states_.geometry, this->track_slot_id()};
}

//---------------------------------------------------------------------------//
/*!
 * Return a geometry-material view.
 */
CELER_FUNCTION auto CoreTrackView::geo_material() const -> GeoMaterialView
{
    return GeoMaterialView{params_.geo_mats};
}

//---------------------------------------------------------------------------//
/*!
 * Return a material view.
 */
CELER_FUNCTION auto CoreTrackView::material() const -> MaterialTrackView
{
    return MaterialTrackView{
        params_.materials, states_.materials, this->track_slot_id()};
}

//---------------------------------------------------------------------------//
/*!
 * Return a particle view.
 */
CELER_FUNCTION auto CoreTrackView::particle() const -> ParticleTrackView
{
    return ParticleTrackView{
        params_.particles, states_.particles, this->track_slot_id()};
}

//---------------------------------------------------------------------------//
/*!
 * Return a particle view of another particle type.
 */
CELER_FUNCTION auto CoreTrackView::particle_record(ParticleId pid) const
    -> ParticleView
{
    return ParticleView{params_.particles, pid};
}

//---------------------------------------------------------------------------//
/*!
 * Return a cutoff view.
 */
CELER_FUNCTION auto CoreTrackView::cutoff() const -> CutoffView
{
    PhysMatId mat_id = this->material().material_id();
    CELER_ASSERT(mat_id);
    return CutoffView{params_.cutoffs, mat_id};
}

//---------------------------------------------------------------------------//
/*!
 * Return a physics view.
 */
CELER_FUNCTION auto CoreTrackView::physics() const -> PhysicsTrackView
{
    PhysMatId mat_id = this->material().material_id();
    CELER_ASSERT(mat_id);
    auto par = this->particle();
    return PhysicsTrackView{
        params_.physics, states_.physics, par, mat_id, this->track_slot_id()};
}

//---------------------------------------------------------------------------//
/*!
 * Return a physics view.
 */
CELER_FUNCTION auto CoreTrackView::physics_step() const -> PhysicsStepView
{
    return PhysicsStepView{
        params_.physics, states_.physics, this->track_slot_id()};
}

//---------------------------------------------------------------------------//
/*!
 * Return the RNG engine.
 */
CELER_FUNCTION auto CoreTrackView::rng() const -> RngEngine
{
    return RngEngine{params_.rng, states_.rng, this->track_slot_id()};
}

//---------------------------------------------------------------------------//
/*!
 * Get the index of the current thread in the current kernel.
 *
 * \warning If the kernel calling this function is not applied to \em all
 * tracks, then comparing against a particular thread ID (e.g. zero for a
 * once-per-kernel initialization) may result in an error.
 *
 * \pre The thread ID is only set if the class is initialized with the thread
 * ID (e.g. from \c TrackExecutor ), which is not the case in track
 * initialization (where the "core track" is constructed from a vacancy).
 */
CELER_FORCEINLINE_FUNCTION ThreadId CoreTrackView::thread_id() const
{
    CELER_EXPECT(thread_id_);
    return thread_id_;
}

//---------------------------------------------------------------------------//
/*!
 * Get the track's index among the states.
 */
CELER_FORCEINLINE_FUNCTION TrackSlotId CoreTrackView::track_slot_id() const
{
    return track_slot_id_;
}

//---------------------------------------------------------------------------//
/*!
 * Get the action ID for encountering a geometry boundary.
 */
CELER_FUNCTION ActionId CoreTrackView::boundary_action() const
{
    return params_.scalars.boundary_action;
}

//---------------------------------------------------------------------------//
/*!
 * Get the action ID for having to pause the step during propagation.
 *
 * This could be from an internal limiter (number of substeps during field
 * propagation) or from having to "bump" the track position for some reason
 * (geometry issue). The volume *must not* change as a result of the
 * propagation, and this should be an extremely rare case.
 */
CELER_FUNCTION ActionId CoreTrackView::propagation_limit_action() const
{
    return params_.scalars.propagation_limit_action;
}

//---------------------------------------------------------------------------//
/*!
 * Get the action ID for killing a track prematurely.
 *
 * This \em unphysical local energy deposition can happen due to:
 * - Initialization in an invalid region
 * - Looping in a magnetic field
 * - A tracking error due to an invalid user geometry or a bug
 * - User tracking cuts
 */
CELER_FUNCTION ActionId CoreTrackView::tracking_cut_action() const
{
    return params_.scalars.tracking_cut_action;
}

//---------------------------------------------------------------------------//
/*!
 * Get access to all the core scalars.
 *
 * TODO: maybe have a struct for all actions to simplify the class? (Action
 * view?)
 */
CELER_FUNCTION CoreScalars const& CoreTrackView::core_scalars() const
{
    return params_.scalars;
}

//---------------------------------------------------------------------------//
/*!
 * Set the 'errored' flag and tracking cut post-step action.
 *
 * \pre This cannot be applied if the current action is *after* post-step. (You
 * can't guarantee for example that sensitive detectors will pick up the energy
 * deposition.)
 */
CELER_FUNCTION void CoreTrackView::apply_errored()
{
    auto sim = this->sim();
    CELER_EXPECT(is_track_valid(sim.status()));
    sim.status(TrackStatus::errored);
    sim.along_step_action({});
    sim.post_step_action(this->tracking_cut_action());
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
