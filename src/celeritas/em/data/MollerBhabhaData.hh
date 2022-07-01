//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/em/data/MollerBhabhaData.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Macros.hh"
#include "corecel/Types.hh"
#include "celeritas/Types.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Model and particles IDs for Moller Bhabha.
 */
struct MollerBhabhaIds
{
    ActionId   action;
    ParticleId electron;
    ParticleId positron;

    //! Whether the IDs are assigned
    explicit CELER_FUNCTION operator bool() const
    {
        return action && electron && positron;
    }
};

//---------------------------------------------------------------------------//
/*!
 * Device data for creating an interactor.
 */
struct MollerBhabhaData
{
    //! Model and particle IDs
    MollerBhabhaIds ids;

    //! Electron mass * c^2 [MeV]
    real_type electron_mass_c_sq;

    //! Model's mininum energy limit [MeV]
    static CELER_CONSTEXPR_FUNCTION real_type min_valid_energy()
    {
        return 1e-3;
    }
    //! Model's maximum energy limit [MeV]
    static CELER_CONSTEXPR_FUNCTION real_type max_valid_energy()
    {
        return 100e6;
    }

    //! Whether the data are assigned
    explicit CELER_FUNCTION operator bool() const
    {
        return ids && electron_mass_c_sq > 0;
    }
};

using MollerBhabhaHostRef   = MollerBhabhaData;
using MollerBhabhaDeviceRef = MollerBhabhaData;

//---------------------------------------------------------------------------//
} // namespace celeritas