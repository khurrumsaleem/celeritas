//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/OpticalCollector.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>

#include "corecel/data/AuxInterface.hh"
#include "celeritas/Types.hh"

#include "OffloadData.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
class ActionRegistry;
class CoreParams;

namespace optical
{
class CerenkovParams;
class MaterialParams;
class ScintillationParams;
}

namespace detail
{
class CerenkovOffloadAction;
class OffloadGatherAction;
class OpticalLaunchAction;
class OffloadParams;
class ScintOffloadAction;
}  // namespace detail

//---------------------------------------------------------------------------//
/*!
 * Generate and track optical photons.
 *
 * This class is the interface between the main stepping loop and the photon
 * stepping loop and constructs kernel actions for:
 * - gathering the pre-step data needed to generate the optical distributions,
 * - generating the scintillation and Cerenkov optical distributions at the
 *   end of the step, and
 * - launching the photon stepping loop.
 *
 * The photon stepping loop will then generate optical primaries.
 *
 * The "collector" (TODO: rename?) will "own" the optical state data and
 * optical params since it's the only thing that launches the optical stepping
 * loop.
 *
 * \todo Rename to OpticalOffload
 */
class OpticalCollector
{
  public:
    //!@{
    //! \name Type aliases
    using SPConstCerenkov = std::shared_ptr<optical::CerenkovParams const>;
    using SPConstMaterial = std::shared_ptr<optical::MaterialParams const>;
    using SPConstScintillation
        = std::shared_ptr<optical::ScintillationParams const>;
    //!@}

    struct Input
    {
        //! Optical physics material for materials
        SPConstMaterial material;
        SPConstCerenkov cerenkov;
        SPConstScintillation scintillation;

        //! Number of steps that have created optical particles
        size_type buffer_capacity{};

        //! True if all input is assigned and valid
        explicit operator bool() const
        {
            return material && (scintillation || cerenkov)
                   && buffer_capacity > 0;
        }
    };

  public:
    // Construct with core data and optical params
    OpticalCollector(CoreParams const&, Input&&);

    // Aux ID for optical offload data
    AuxId offload_aux_id() const;

  private:
    //// TYPES ////

    using SPOffloadParams = std::shared_ptr<detail::OffloadParams>;
    using SPCerenkovAction = std::shared_ptr<detail::CerenkovOffloadAction>;
    using SPScintAction = std::shared_ptr<detail::ScintOffloadAction>;
    using SPGatherAction = std::shared_ptr<detail::OffloadGatherAction>;
    using SPLaunchAction = std::shared_ptr<detail::OpticalLaunchAction>;

    //// DATA ////

    SPOffloadParams gen_params_;

    SPGatherAction gather_action_;
    SPCerenkovAction cerenkov_action_;
    SPScintAction scint_action_;
    SPLaunchAction launch_action_;

    // TODO: tracking loop launch action
};

//---------------------------------------------------------------------------//
}  // namespace celeritas
