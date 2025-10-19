//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/OpticalCollector.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <optional>

#include "corecel/math/NumericLimits.hh"
#include "celeritas/Types.hh"
#include "celeritas/optical/Types.hh"
#include "celeritas/phys/GeneratorCounters.hh"
#include "celeritas/phys/GeneratorRegistry.hh"

#include "Model.hh"
#include "gen/OffloadData.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
class ActionRegistry;
class AuxStateVec;
class CherenkovParams;
class CoreParams;
class CoreStateInterface;
template<GeneratorType G>
class OffloadAction;
class OffloadGatherAction;
class ScintillationParams;

namespace optical
{
class CoreStateBase;
class GeneratorAction;
class MaterialParams;
}  // namespace optical

namespace detail
{
class OpticalLaunchAction;
}  // namespace detail

//---------------------------------------------------------------------------//
/*!
 * Generate and track optical photons.
 *
 * This class is the interface between the main stepping loop and the photon
 * stepping loop and constructs kernel actions for:
 * - gathering the pre-step data needed to generate the optical distributions,
 * - generating the scintillation and Cherenkov optical distributions at the
 *   end of the step, and
 * - launching the photon stepping loop.
 *
 * The photon stepping loop will then generate optical primaries.
 *
 * The "collector" (TODO: rename?) will "own" the optical state data and
 * optical params since it's the only thing that launches the optical stepping
 * loop.
 *
 * \todo This doesn't do anything but set up the optical tracking loop: move to
 * \c setup namespace
 */
class OpticalCollector
{
  public:
    //!@{
    //! \name Type aliases
    using SPConstCherenkov = std::shared_ptr<CherenkovParams const>;
    using SPConstMaterial = std::shared_ptr<optical::MaterialParams const>;
    using SPConstScintillation = std::shared_ptr<ScintillationParams const>;
    using OpticalBufferSize = GeneratorCounters<size_type>;
    using SPConstOpticalParams = std::shared_ptr<optical::CoreParams const>;
    //!@}

    struct Input
    {
        //! Optical params
        std::shared_ptr<optical::CoreParams> optical_params;

        //! Optical photon generating processes
        SPConstCherenkov cherenkov;
        SPConstScintillation scintillation;

        //! Number track slots in the optical loop
        size_type num_track_slots{};

        //! Number of steps that have created optical particles
        size_type buffer_capacity{};

        //! Threshold number of photons for launching optical loop
        size_type auto_flush{};

        //! Maximum step iterations before aborting optical loop
        size_type max_step_iters{numeric_limits<size_type>::max()};

        //! True if all input is assigned and valid
        explicit operator bool() const
        {
            return optical_params && (scintillation || cherenkov)
                   && num_track_slots > 0 && buffer_capacity > 0
                   && auto_flush > 0;
        }
    };

  public:
    // Construct with core data and optical params
    OpticalCollector(CoreParams const&, Input&&);

    //// ACCESSORS ////

    //! Access optical params
    SPConstOpticalParams const& optical_params() const
    {
        return optical_params_;
    }

    // Access optical state
    optical::CoreStateBase const&
    optical_state(CoreStateInterface const& core) const;

    // Access Cherenkov params (may be null)
    SPConstCherenkov cherenkov() const;

    // Access scintillation params (may be null)
    SPConstScintillation scintillation() const;

    //// GENERATOR MANAGEMENT ////

    // Get the generator registry
    GeneratorRegistry const& gen_reg() const;

    // Get and reset cumulative statistics on optical tracks from a state
    OpticalAccumStats exchange_counters(AuxStateVec& aux) const;

    // Get queued buffer sizes
    OpticalBufferSize buffer_counts(AuxStateVec const& aux) const;

  private:
    //// TYPES ////

    using GT = GeneratorType;
    using SPCherenkovOffload = std::shared_ptr<OffloadAction<GT::cherenkov>>;
    using SPScintOffload = std::shared_ptr<OffloadAction<GT::scintillation>>;
    using SPGatherAction = std::shared_ptr<OffloadGatherAction>;
    using SPGenerator = std::shared_ptr<optical::GeneratorAction>;
    using SPLaunchAction = std::shared_ptr<detail::OpticalLaunchAction>;

    //// DATA ////

    SPConstOpticalParams optical_params_;
    SPGatherAction gather_;
    SPCherenkovOffload cherenkov_offload_;
    SPScintOffload scint_offload_;
    SPGenerator generate_;
    SPLaunchAction launch_;
};

//---------------------------------------------------------------------------//
}  // namespace celeritas
