//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file orange/transform/NoTransformation.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Macros.hh"
#include "corecel/cont/Span.hh"
#include "orange/OrangeTypes.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Apply an identity transformation.
 *
 * This trivial class has the Transformation interface but has no storage
 * requirements and simply passes through all its data.
 */
class NoTransformation
{
  public:
    //@{
    //! \name Type aliases
    using StorageSpan = Span<real_type const, 0>;
    //@}

    //! Transform type identifier
    static CELER_CONSTEXPR_FUNCTION TransformType transform_type()
    {
        return TransformType::no_transformation;
    }

  public:
    //! Construct with an identity NoTransformation
    NoTransformation() = default;

    //! Construct inline from storage
    explicit CELER_FUNCTION NoTransformation(StorageSpan) {}

    //// ACCESSORS ////

    //! Get a view to the data for type-deleted storage
    CELER_FUNCTION StorageSpan data() const { return {}; }

    //// CALCULATION ////

    //! Transform from daughter to parent (identity)
    CELER_FUNCTION Real3 const& transform_up(Real3 const& d) const
    {
        return d;
    }

    //! Transform from parent to daughter (identity)
    CELER_FUNCTION Real3 const& transform_down(Real3 const& d) const
    {
        return d;
    }

    //! Rotate from daughter to parent (identity)
    CELER_FUNCTION Real3 const& rotate_up(Real3 const& d) const { return d; }

    //! Rotate from parent to daughter (identity)
    CELER_FUNCTION Real3 const& rotate_down(Real3 const& d) const { return d; }

    // Calculate the inverse during preprocessing
    inline NoTransformation calc_inverse() const { return {}; }
};

//---------------------------------------------------------------------------//
// FREE FUNCTIONS
//---------------------------------------------------------------------------//
//!@{
//! Host-only comparators
inline constexpr bool
operator==(NoTransformation const&, NoTransformation const&)
{
    return true;
}

inline constexpr bool
operator!=(NoTransformation const&, NoTransformation const&)
{
    return false;
}
//!@}

//---------------------------------------------------------------------------//
}  // namespace celeritas
