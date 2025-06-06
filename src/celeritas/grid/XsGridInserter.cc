//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/grid/XsGridInserter.cc
//---------------------------------------------------------------------------//
#include "XsGridInserter.hh"

#include "corecel/Types.hh"

#include "XsGridData.hh"

#include "detail/GridUtils.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Construct with a reference to mutable host data.
 */
XsGridInserter::XsGridInserter(Values* reals, GridValues* grids)
    : values_(reals), reals_(reals), grids_(grids)
{
    CELER_EXPECT(reals && grids);
}

//---------------------------------------------------------------------------//
/*!
 * Add a grid of physics xs data.
 */
auto XsGridInserter::operator()(inp::XsGrid const& xs) -> GridId
{
    CELER_EXPECT(xs);

    XsGridRecord grid;
    if (auto const& lower = xs.lower)
    {
        grid.lower.grid = UniformGridData::from_bounds(lower.x, lower.y.size());
        grid.lower.value = reals_.insert_back(lower.y.begin(), lower.y.end());
        detail::set_spline(values_, reals_, lower.interpolation, grid.lower);
    }
    if (auto const& upper = xs.upper)
    {
        grid.upper.grid = UniformGridData::from_bounds(upper.x, upper.y.size());
        grid.upper.value = reals_.insert_back(upper.y.begin(), upper.y.end());
        detail::set_spline(values_, reals_, upper.interpolation, grid.upper);
    }
    CELER_ENSURE(grid);
    return grids_.push_back(grid);
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
