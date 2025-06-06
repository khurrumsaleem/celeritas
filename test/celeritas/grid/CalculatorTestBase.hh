//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/grid/CalculatorTestBase.hh
//---------------------------------------------------------------------------//
#pragma once

#include <vector>

#include "corecel/cont/Array.hh"
#include "corecel/cont/Span.hh"
#include "corecel/data/Collection.hh"
#include "corecel/grid/SplineDerivCalculator.hh"
#include "celeritas/grid/XsGridData.hh"
#include "celeritas/inp/Grid.hh"

#include "Test.hh"

namespace celeritas
{
namespace test
{
//---------------------------------------------------------------------------//
/*!
 * Test harness base class for interpolating values on grids.
 */
class CalculatorTestBase : public Test
{
  public:
    //!@{
    //! \name Type aliases
    using BC = SplineDerivCalculator::BoundaryCondition;
    using Values = Collection<real_type, Ownership::value, MemSpace::host>;
    using Data
        = Collection<real_type, Ownership::const_reference, MemSpace::host>;
    //!@}

  public:
    // Construct a cross section grid
    void build(inp::XsGrid grid);

    // Construct a uniform grid
    void build(inp::UniformGrid grid);

    // Construct an inverted uniform grid
    void build_inverted(inp::UniformGrid grid);

    XsGridRecord const& xs_grid() const { return data_; }
    UniformGridRecord const& uniform_grid() const { return data_.lower; }
    Data const& values() const { return value_ref_; }

  private:
    XsGridRecord data_;
    Values value_storage_;
    Data value_ref_;

    void build_impl(inp::UniformGrid, inp::UniformGrid, bool invert);
    void build_grid(UniformGridRecord&, inp::UniformGrid const&, bool invert);
};

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace celeritas
