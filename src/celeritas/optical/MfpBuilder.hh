//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/MfpBuilder.hh
//---------------------------------------------------------------------------//
#pragma once

#include "celeritas/grid/NonuniformGridInserter.hh"

namespace celeritas
{
namespace optical
{
//---------------------------------------------------------------------------//
/*!
 * Helper class for optical models to build MFP tables.
 *
 * Tracks individual grid IDs that have been built, and returns them
 * as an ItemRange which may be used by model MFP tables.
 */
class MfpBuilder
{
  public:
    //!@{
    //! \name Type aliases
    using GridId = OpaqueId<NonuniformGridRecord>;
    using GridInserter = NonuniformGridInserter<GridId>;
    using GridIdRange = Range<GridId>;

    using Values = typename GridInserter::Values;
    using GridValues = typename GridInserter::GridValues;
    //!@}

  public:
    // Construct with given inserter
    inline MfpBuilder(Values* real_data, GridValues* grid_data);

    // Build the grid
    template<typename... Args>
    inline void operator()(Args const&... args);

    // Get the range of grid IDs that have been built
    inline GridIdRange grid_ids() const;

  private:
    GridInserter insert_grid_;
    GridValues* grid_data_;
    GridId const grid_id_first_;
};

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Construct with given collections.
 */
MfpBuilder::MfpBuilder(Values* real_data, GridValues* grid_data)
    : insert_grid_(real_data, grid_data)
    , grid_data_(grid_data)
    , grid_id_first_(grid_data->size())
{
}

//---------------------------------------------------------------------------//
/*!
 * Build the grid.
 *
 * Passes its arguments directly to a \c NonuniformGridInserter.
 */
template<typename... Args>
void MfpBuilder::operator()(Args const&... args)
{
    insert_grid_(args...);
}

//---------------------------------------------------------------------------//
/*!
 * Get the range of grid IDs that have been built.
 */
auto MfpBuilder::grid_ids() const -> GridIdRange
{
    return GridIdRange(grid_id_first_, GridId{grid_data_->size()});
}

//---------------------------------------------------------------------------//
}  // namespace optical
}  // namespace celeritas
