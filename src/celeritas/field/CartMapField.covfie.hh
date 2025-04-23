//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/field/CartMapField.covfie.hh
//---------------------------------------------------------------------------//
#pragma once

#include <type_traits>

#include "corecel/Macros.hh"
#include "corecel/Types.hh"
#include "corecel/cont/Array.hh"
#include "corecel/cont/Range.hh"
#include "celeritas/Types.hh"
#include "celeritas/field/CartMapFieldData.hh"

#include "detail/CovfieFieldTraits.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Interpolate a magnetic field vector on an x/y/z grid.
 */
class CartMapField
{
  public:
    //!@{
    //! \name Type aliases
    using real_type = float;
    using Real3 = Array<celeritas::real_type, 3>;
    using FieldParamsRef = NativeCRef<CartMapFieldParamsData>;
    //!@}

  public:
    // Construct with the shared map data
    inline CELER_FUNCTION explicit CartMapField(FieldParamsRef const& shared);

    // Evaluate the magnetic field value for the given position
    CELER_FUNCTION
    inline Real3 operator()(Real3 const& pos) const;

  private:
    using field_view_t = FieldParamsRef::view_t;
    field_view_t const& field_;
};

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Construct with the shared magnetic field map data.
 */
CELER_FUNCTION
CartMapField::CartMapField(FieldParamsRef const& shared)
    : field_{shared.get_view()}
{
}

//---------------------------------------------------------------------------//
/*!
 * Calculate the magnetic field vector for the given position.
 *
 * This does a 3-D interpolation on the input grid and reconstructs the
 * magnetic field vector from the stored X, Y, Z components of the field.
 * The result is in the native Celeritas unit system.
 */
CELER_FUNCTION auto CartMapField::operator()(Real3 const& pos) const -> Real3
{
    return detail::CovfieFieldTraits<MemSpace::native>::to_array(
        field_.at(static_cast<real_type>(pos[0]),
                  static_cast<real_type>(pos[1]),
                  static_cast<real_type>(pos[2])));
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
