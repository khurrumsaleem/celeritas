//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/em/data/ElectronBremsData.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Macros.hh"
#include "celeritas/Types.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
//! IDs used by brems
struct ElectronBremIds
{
    //! ID of a gamma
    ParticleId gamma;
    //! ID of an electron
    ParticleId electron;
    //! ID of an positron
    ParticleId positron;

    //! Whether the IDs are assigned
    explicit CELER_FUNCTION operator bool() const
    {
        return gamma && electron && positron;
    }
};

//---------------------------------------------------------------------------//
}  // namespace celeritas
