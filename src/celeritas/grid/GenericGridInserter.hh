//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/grid/GenericGridInserter.hh
//---------------------------------------------------------------------------//
#pragma once

#include <utility>
#include <vector>

#include "corecel/Types.hh"
#include "corecel/cont/Span.hh"
#include "corecel/data/Collection.hh"
#include "corecel/data/CollectionBuilder.hh"
#include "celeritas/Types.hh"
#include "celeritas/io/ImportPhysicsVector.hh"

#include "GenericGridBuilder.hh"
#include "GenericGridData.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Construct a generic grid using mutable host data and add it to
 * the specified grid collection.
 *
 * \code
    GenericGridInserter insert(&data->reals, &data->generic_grids);
    std::vector<GenericGridIndex> grid_ids;
    for (material : range(MaterialId{mats->size()}))
        grid_ids.push_back(insert(physics_vector[material.get()]));
   \endcode
 */
template<class Index>
class GenericGridInserter
{
  public:
    //!@{
    //! \name Type aliases
    using SpanConstFlt = Span<float const>;
    using SpanConstDbl = Span<double const>;
    using RealCollection
        = Collection<real_type, Ownership::value, MemSpace::host>;
    using GenericGridCollection
        = Collection<GenericGridRecord, Ownership::value, MemSpace::host, Index>;
    //!@}

  public:
    // Construct with a reference to mutable host data
    GenericGridInserter(RealCollection* real_data, GenericGridCollection* grid);

    // Add a grid of generic data with linear interpolation
    Index operator()(SpanConstFlt grid, SpanConstFlt values);

    // Add a grid of generic data with linear interpolation
    Index operator()(SpanConstDbl grid, SpanConstDbl values);

    // Add an imported physics vector as a grid
    Index operator()(ImportPhysicsVector const& vec);

    // Add an empty grid (no data present)
    Index operator()();

  private:
    GenericGridBuilder grid_builder_;
    CollectionBuilder<GenericGridRecord, MemSpace::host, Index> grids_;
};

//---------------------------------------------------------------------------//
/*!
 * Construct with a reference to mutable host data.
 */
template<class Index>
GenericGridInserter<Index>::GenericGridInserter(RealCollection* real_data,
                                                GenericGridCollection* grid)
    : grid_builder_(real_data), grids_(grid)
{
    CELER_EXPECT(real_data && grid);
}

//---------------------------------------------------------------------------//
/*!
 * Add an imported physics vector as a generic grid to the collection.
 *
 * Returns the id of the inserted grid, or an empty id if the vector is
 * empty.
 */
template<class Index>
auto GenericGridInserter<Index>::operator()(ImportPhysicsVector const& vec) -> Index
{
    CELER_EXPECT(!vec.x.empty());
    return grids_.push_back(grid_builder_(vec));
}

//---------------------------------------------------------------------------//
/*!
 * Add a grid of generic data with linear interpolation to the collection.
 */
template<class Index>
auto GenericGridInserter<Index>::operator()(SpanConstFlt grid,
                                            SpanConstFlt values) -> Index
{
    CELER_EXPECT(!grid.empty());
    return grids_.push_back(grid_builder_(grid, values));
}

//---------------------------------------------------------------------------//
/*!
 * Add a grid of generic data with linear interpolation to the collection.
 */
template<class Index>
auto GenericGridInserter<Index>::operator()(SpanConstDbl grid,
                                            SpanConstDbl values) -> Index
{
    CELER_EXPECT(!grid.empty());
    return grids_.push_back(grid_builder_(grid, values));
}

//---------------------------------------------------------------------------//
/*!
 * Add an empty grid.
 *
 * Useful for when there's no imported grid present for a given material.
 */
template<class Index>
auto GenericGridInserter<Index>::operator()() -> Index
{
    return grids_.push_back({});
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
