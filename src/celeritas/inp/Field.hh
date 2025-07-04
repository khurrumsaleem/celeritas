//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/inp/Field.hh
//---------------------------------------------------------------------------//
#pragma once

#include <unordered_set>
#include <variant>

#include "geocel/Types.hh"
#include "celeritas/UnitTypes.hh"
#include "celeritas/field/CartMapFieldInput.hh"
#include "celeritas/field/CylMapFieldInput.hh"
#include "celeritas/field/FieldDriverOptions.hh"
#include "celeritas/field/RZMapFieldInput.hh"

class G4LogicalVolume;

namespace celeritas
{
namespace inp
{
//---------------------------------------------------------------------------//
/*!
 * Build a problem without magnetic fields.
 */
struct NoField
{
};

//---------------------------------------------------------------------------//
/*!
 * Create a uniform nonzero field.
 *
 * If volumes are specified, the field will only be present in those volumes.
 *
 * \todo Field driver options will be separate from the magnetic field. They,
 * plus the field type, will be specified in a FieldParams that maps
 * region/particle/energy to field setup. NOTE ALSO that \c
 * driver_options.max_substeps is redundant with \c
 * p.tracking.limits.field_substeps .
 */
struct UniformField
{
    using SetVolume = std::unordered_set<G4LogicalVolume const*>;
    using SetString = std::unordered_set<std::string>;
    using VariantSetVolume = std::variant<std::monostate, SetVolume, SetString>;

    //! Default field units are tesla
    UnitSystem units{UnitSystem::si};

    //! Field strength
    Real3 strength{0, 0, 0};

    //! Field driver options
    FieldDriverOptions driver_options;

    //! Volumes where the field is present (optional)
    VariantSetVolume volumes;
};

//---------------------------------------------------------------------------//
/*!
 * Build a separable R-Z magnetic field from a file.
 *
 * \todo v0.7 Move field input here
 */
using RZMapField = ::celeritas::RZMapFieldInput;
using CylMapField = ::celeritas::CylMapFieldInput;
using CartMapField = ::celeritas::CartMapFieldInput;

//---------------------------------------------------------------------------//
//! Field type
using Field
    = std::variant<NoField, UniformField, RZMapField, CylMapField, CartMapField>;

//---------------------------------------------------------------------------//
}  // namespace inp
}  // namespace celeritas
