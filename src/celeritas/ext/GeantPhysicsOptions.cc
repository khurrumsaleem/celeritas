//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/ext/GeantPhysicsOptions.cc
//---------------------------------------------------------------------------//
#include "GeantPhysicsOptions.hh"

#include "corecel/io/EnumStringMapper.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Get a string corresponding to the Bremsstrahlung model selection.
 */
char const* to_cstring(BremsModelSelection value)
{
    static EnumStringMapper<BremsModelSelection> const to_cstring_impl{
        "none",
        "seltzer_berger",
        "relativistic",
        "all",
    };
    return to_cstring_impl(value);
}

//---------------------------------------------------------------------------//
/*!
 * Get a string corresponding to the multiple scattering model selection.
 */
char const* to_cstring(MscModelSelection value)
{
    static EnumStringMapper<MscModelSelection> const to_cstring_impl{
        "none",
        "urban",
        "wentzelvi",
        "urban_wentzelvi",
    };
    return to_cstring_impl(value);
}

//---------------------------------------------------------------------------//
/*!
 * Get a string corresponding to the atomic relaxation option.
 */
char const* to_cstring(RelaxationSelection value)
{
    static EnumStringMapper<RelaxationSelection> const to_cstring_impl{
        "none",
        "radiative",
        "all",
    };
    return to_cstring_impl(value);
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
