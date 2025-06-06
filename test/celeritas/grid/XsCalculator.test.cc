//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/grid/XsCalculator.test.cc
//---------------------------------------------------------------------------//
#include "celeritas/grid/XsCalculator.hh"

#include <algorithm>
#include <cmath>
#include <vector>

#include "corecel/cont/Range.hh"
#include "corecel/data/CollectionBuilder.hh"
#include "corecel/io/Repr.hh"

#include "CalculatorTestBase.hh"
#include "celeritas_test.hh"

namespace celeritas
{
namespace test
{
//---------------------------------------------------------------------------//
// TEST HARNESS
//---------------------------------------------------------------------------//

class XsCalculatorTest : public CalculatorTestBase
{
  protected:
    using Energy = XsCalculator::Energy;
};

//---------------------------------------------------------------------------//
// TESTS
//---------------------------------------------------------------------------//

TEST_F(XsCalculatorTest, simple)
{
    // Energy from 1 to 1e5 MeV with 6 grid points; XS = E
    // *No* magical 1/E scaling
    inp::XsGrid grid;
    grid.lower.x = {1, 1e5};
    grid.lower.y = {1, 10, 1e2, 1e3, 1e4, 1e5};
    this->build(grid);

    XsCalculator calc(this->xs_grid(), this->values());

    // Test on grid points
    EXPECT_SOFT_EQ(1.0, calc(Energy{1}));
    EXPECT_SOFT_EQ(1e2, calc(Energy{1e2}));
    EXPECT_SOFT_EQ(1e5 - 1e-6, calc(Energy{1e5 - 1e-6}));
    EXPECT_SOFT_EQ(1e5, calc(Energy{1e5}));

    // Test between grid points
    EXPECT_SOFT_EQ(5, calc(Energy{5}));

    // Test out-of-bounds
    EXPECT_SOFT_EQ(1.0, calc(Energy{0.0001}));
    EXPECT_SOFT_EQ(1e5, calc(Energy{1e7}));

    // Test energy grid bounds
    EXPECT_SOFT_EQ(1.0, value_as<Energy>(calc.energy_min()));
    EXPECT_SOFT_EQ(1e5, value_as<Energy>(calc.energy_max()));
}

TEST_F(XsCalculatorTest, scaled_lowest)
{
    // Energy from .1 to 1e4 MeV with 6 grid points and values of 1
    inp::XsGrid grid;
    grid.upper.x = {0.1, 1e4};
    grid.upper.y = {1, 1, 1, 1, 1, 1};
    this->build(grid);

    XsCalculator calc(this->xs_grid(), this->values());

    // Test on grid points
    EXPECT_SOFT_EQ(1, calc(Energy{0.1}));
    EXPECT_SOFT_EQ(1, calc(Energy{1e2}));
    EXPECT_SOFT_EQ(1, calc(Energy{1e4 - 1e-6}));
    EXPECT_SOFT_EQ(1, calc(Energy{1e4}));

    // Test between grid points
    EXPECT_SOFT_EQ(1, calc(Energy{0.2}));
    EXPECT_SOFT_EQ(1, calc(Energy{5}));

    // Test out-of-bounds: cross section still scales according to 1/E (TODO:
    // this might not be the best behavior for the lower energy value)
    EXPECT_SOFT_EQ(1000, calc(Energy{0.0001}));
    EXPECT_SOFT_EQ(0.1, calc(Energy{1e5}));

    // Test energy grid bounds
    EXPECT_SOFT_EQ(0.1, value_as<Energy>(calc.energy_min()));
    EXPECT_SOFT_EQ(1e4, value_as<Energy>(calc.energy_max()));
}

TEST_F(XsCalculatorTest, scaled_middle)
{
    // Energy from .1 to 1e4 MeV with 6 grid points
    inp::XsGrid grid;
    grid.lower.x = {0.1, 10};
    grid.lower.y = {3, 3, 3};
    grid.upper.x = {grid.lower.x[Bound::hi], 1e4};
    grid.upper.y = {3, 3, 3, 3};
    this->build(grid);

    XsCalculator calc(this->xs_grid(), this->values());

    // Test on grid points
    EXPECT_SOFT_EQ(3, calc(Energy{0.1}));
    EXPECT_SOFT_EQ(3, calc(Energy{1e2}));
    EXPECT_SOFT_EQ(3, calc(Energy{1e4 - 1e-6}));
    EXPECT_SOFT_EQ(3, calc(Energy{1e4}));

    // Test between grid points
    EXPECT_SOFT_EQ(3, calc(Energy{0.2}));
    EXPECT_SOFT_EQ(3, calc(Energy{5}));

    // Test out-of-bounds: cross section still scales according to 1/E
    EXPECT_SOFT_EQ(3, calc(Energy{0.0001}));
    EXPECT_SOFT_EQ(0.3, calc(Energy{1e5}));

    // Test energy grid bounds
    EXPECT_SOFT_EQ(0.1, value_as<Energy>(calc.energy_min()));
    EXPECT_SOFT_EQ(1e4, value_as<Energy>(calc.energy_max()));
}

TEST_F(XsCalculatorTest, scaled_linear)
{
    auto xs = [](real_type energy) {
        auto result = 100 + energy * 10;
        if (energy > 1)
        {
            result *= 1 / energy;
        }
        return result;
    };

    inp::XsGrid grid;
    grid.lower.x = {1e-3, 1};
    grid.lower.y = {xs(1e-3), xs(1e-2), xs(1e-1), xs(1)};
    grid.upper.x = {grid.lower.x[Bound::hi], 1e3};
    grid.upper.y = {xs(1), xs(1e1), xs(1e2), xs(1e3)};
    this->build(grid);

    XsCalculator interp_xs(this->xs_grid(), this->values());

    for (real_type e : {1e-3, 1e-1, 0.5, 1.0, 1.5, 10.0, 12.5, 1e3})
    {
        EXPECT_SOFT_EQ(xs(e), interp_xs(Energy{e})) << "e=" << repr(e);
    }
}

TEST_F(XsCalculatorTest, scaled_highest)
{
    // values of 1, 10, 100 --> actual xs = {1, 10, 1}
    inp::XsGrid grid;
    grid.lower.x = {1, 100};
    grid.lower.y = {1, 10, 1};
    grid.upper.x = {grid.lower.x[Bound::hi], 100};
    grid.upper.y = {1, 1};

    // Can't have a single scaled value; only the lower grid is built
    this->build(grid);
    EXPECT_TRUE(this->xs_grid().lower);
    EXPECT_FALSE(this->xs_grid().upper);
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace celeritas
