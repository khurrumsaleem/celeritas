//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/inp/Physics.hh
//---------------------------------------------------------------------------//
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "corecel/Types.hh"
#include "celeritas/Types.hh"
#include "celeritas/phys/AtomicNumber.hh"

#include "PhysicsProcess.hh"

namespace celeritas
{
namespace inp
{
//---------------------------------------------------------------------------//
/*!
 * Electromagnetic physics processes and options.
 */
struct EmPhysics
{
    //! Bremsstrahlung process
    std::optional<BremsProcess> brems{std::in_place};
    //! Electron+positron pair production process
    std::optional<PairProductionProcess> pair_production{std::in_place};

    //!@{
    //! \name Energy loss and slowing down

    //! Use spline interpolation for energy loss
    size_type eloss_spline_order{1};

    // TODO: currently eloss fluctuations are set up via geant importer, then
    // read into ImportEmParams
#if 0
     //! Energy loss fluctuations
     bool eloss_fluct{true};
#endif
    //
    //!@}
};

//---------------------------------------------------------------------------//
/*!
 * Optical physics processes and options.
 */
struct OpticalPhysics
{
};

//---------------------------------------------------------------------------//
/*!
 * Hadronic physics processes and options.
 *
 * This can be used to enable or set up Geant4 hadronic physics.
 */
struct HadronicPhysics
{
};

//---------------------------------------------------------------------------//
/*!
 * Decay processes and options.
 */
struct DecayPhysics
{
};

//---------------------------------------------------------------------------//
/*!
 * Set up physics options.
 *
 * \todo Move optical and hadronic physics options from
 *       \c celeritas::GeantPhysicsOptions
 * \todo Move particle data from \c celeritas::ImportParticle
 * \todo Add function for injecting user processes for
 *       \c celeritas::PhysicsParams
 */
struct Physics
{
    //! Enable electromagnetic physics
    std::optional<EmPhysics> em{std::in_place};

    //! Enable optical photon physics
    std::optional<OpticalPhysics> optical;

    //! Enable hadronic physics
    std::optional<HadronicPhysics> hadronic;

    //! Enable decay physics
    std::optional<DecayPhysics> decay;
};

//---------------------------------------------------------------------------//
}  // namespace inp
}  // namespace celeritas
