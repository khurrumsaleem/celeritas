//----------------------------------*-C++-*----------------------------------//
// Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/ext/GeantUtils.hh
//---------------------------------------------------------------------------//
#pragma once

#include "celeritas_config.h"
#include "corecel/Assert.hh"

class G4RunManager;

namespace celeritas
{
//---------------------------------------------------------------------------//
// Get the number of threads in a version-portable way
int get_geant_num_threads(G4RunManager const&);

//---------------------------------------------------------------------------//
// Get the current thread ID (zero if serial)
int get_geant_thread_id();

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
#if !CELERITAS_USE_GEANT4
inline int get_geant_num_threads(G4RunManager const&)
{
    CELER_NOT_CONFIGURED("Geant4");
}

inline int get_geant_thread_id()
{
    CELER_NOT_CONFIGURED("Geant4");
}
#endif
//---------------------------------------------------------------------------//
}  // namespace celeritas