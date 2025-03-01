//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/field/UniformFieldData.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Types.hh"
#include "corecel/cont/Array.hh"

#include "FieldDriverOptions.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Input data and options for a uniform field.
 */
struct UniformFieldParams
{
    using Real3 = Array<real_type, 3>;

    Real3 field{0, 0, 0};  //!< Field strength (native units)
    FieldDriverOptions options;
};

//---------------------------------------------------------------------------//
}  // namespace celeritas
