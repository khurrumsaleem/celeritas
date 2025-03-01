//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file orange/orangeinp/detail/SurfaceGridHash.hh
//---------------------------------------------------------------------------//
#pragma once

#include <cstdlib>
#include <utility>

#include "corecel/Types.hh"
#include "corecel/cont/Array.hh"
#include "orange/OrangeTypes.hh"

namespace celeritas
{
namespace orangeinp
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Hash "similar" surfaces for faster lookups.
 *
 * This is meant to generate one or more "key" values for a hash of surfaces.
 *
 * This creates a hash map of local surfaces based on a characteristic
 * dimension (i.e. the radius of a sphere), which is used to accelerate surface
 * deduplication. For a given surface in bin N, possible duplicates may be
 * found in bins N-1, N, N+1.
 *
 * - Nearby surfaces should always have nearby "hash points", within some
 *   comparison tolerance.
 * - The comparison tolerance must be less than the grid width, probably \em
 *   much less.
 * - Different surfaces can have an identical hash point but have
 *   different surface types.
 * - The bin values will *always* be unique given a surface type.
 *
 * \sa LocalSurfaceInserter
 */
class SurfaceGridHash
{
  public:
    //!@{
    //! \name Type aliases
    using key_type = std::size_t;
    using result_type = Array<key_type, 2>;
    //!@}

  public:
    // Construct with maximum tolerance and characteristic scale of grid
    SurfaceGridHash(real_type grid_scale, real_type tol);

    // Construct keys for the grid
    result_type operator()(SurfaceType type, real_type hash_point) const;

    //! Sentinel value for a hash point being redundant
    static constexpr key_type redundant() { return static_cast<key_type>(-1); }

  private:
    real_type eps_;
    real_type grid_offset_;
    real_type inv_grid_width_;

    // Calculate the bin of a new data point
    key_type calc_bin(SurfaceType type, real_type hash_point) const;
};

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace orangeinp
}  // namespace celeritas
