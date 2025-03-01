//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file corecel/math/Integrator.test.cc
//---------------------------------------------------------------------------//
#include "corecel/math/Integrator.hh"

#include <cmath>

#include "corecel/Config.hh"

#include "DiagnosticRealFunc.hh"
#include "celeritas_test.hh"

namespace celeritas
{
namespace test
{
//---------------------------------------------------------------------------//
TEST(IntegratorTest, constant)
{
    DiagnosticRealFunc f{[](real_type) { return real_type{10}; }};
    {
        Integrator integrate{f};
        EXPECT_SOFT_EQ(10.0, integrate(1, 2));
        EXPECT_EQ(3, f.exchange_count());
        EXPECT_SOFT_EQ(10 * 10.0, integrate(2, 12));
        EXPECT_EQ(3, f.exchange_count());
    }
}

TEST(IntegratorTest, linear)
{
    DiagnosticRealFunc f{[](real_type x) { return 2 * x; }};
    {
        Integrator integrate{f};
        EXPECT_SOFT_EQ(4 - 1, integrate(1, 2));
        EXPECT_EQ(3, f.exchange_count());
        EXPECT_SOFT_EQ(16 - 4, integrate(2, 4));
        EXPECT_EQ(3, f.exchange_count());
    }
}

TEST(IntegratorTest, quadratic)
{
    DiagnosticRealFunc f{[](real_type x) { return 3 * ipow<2>(x); }};
    {
        real_type const eps = IntegratorOptions{}.epsilon;
        Integrator integrate{f};
        EXPECT_SOFT_NEAR(8 - 1, integrate(1, 2), eps);
        EXPECT_EQ(17, f.exchange_count());
        EXPECT_SOFT_NEAR(64 - 8, integrate(2, 4), eps);
        EXPECT_EQ(17, f.exchange_count());
    }
    {
        IntegratorOptions opts;
        opts.epsilon = 1e-5;
        Integrator integrate{f, opts};
        EXPECT_SOFT_NEAR(8 - 1, integrate(1, 2), opts.epsilon);
        EXPECT_EQ(257, f.exchange_count());
        EXPECT_SOFT_NEAR(64 - 8, integrate(2, 4), opts.epsilon);
        EXPECT_EQ(257, f.exchange_count());
    }
}

TEST(IntegratorTest, gauss)
{
    DiagnosticRealFunc f{
        [](real_type r) { return ipow<2>(r) * std::exp(-ipow<2>(r)); }};
    {
        Integrator integrate{f};
        EXPECT_SOFT_EQ(0.057594067180233119, integrate(0, 0.597223));
        EXPECT_EQ(33, f.exchange_count());
        EXPECT_SOFT_EQ(0.16739988271111467, integrate(0.597223, 1.09726));
        EXPECT_EQ(17, f.exchange_count());
        EXPECT_SOFT_EQ(0.20618863449804861, integrate(1.09726, 2.14597));
        EXPECT_EQ(5, f.exchange_count());
    }
    if (CELERITAS_REAL_TYPE == CELERITAS_REAL_TYPE_DOUBLE)
    {
        IntegratorOptions opts;
        opts.epsilon = 1e-8;
        opts.max_depth = 30;
        Integrator integrate{f, opts};
        EXPECT_SOFT_NEAR(
            0.057578453318570512, integrate(0, 0.597223), opts.epsilon);
        EXPECT_EQ(16385, f.exchange_count());
        EXPECT_SOFT_NEAR(
            0.16745460321713002, integrate(0.597223, 1.09726), opts.epsilon);
        EXPECT_EQ(8193, f.exchange_count());
        EXPECT_SOFT_NEAR(
            0.20628439788305011, integrate(1.09726, 2.14597), opts.epsilon);
        EXPECT_EQ(2049, f.exchange_count());
    }
}

/*!
 * Integrate a pathological function.
 *
 * This is disabled because:
 * - The integrated result changes based on the executing system, possibly due
 *   to fma implementation.
 * - There is an overflow or NaN with single precision.
 * - The convergence takes slightly different number of iterations on different
 *   compilers.
 * - The result is wrong anyway.
 */
TEST(IntegratorTest, DISABLED_nasty)
{
    DiagnosticRealFunc f{[](real_type x) { return std::cos(std::exp(1 / x)); }};
    {
        real_type const eps = IntegratorOptions{}.epsilon;
        Integrator integrate{f};
        if (CELERITAS_DEBUG)
        {
            // Out of range
            EXPECT_THROW(integrate(0, 1), DebugError);
        }

        EXPECT_SOFT_NEAR(-0.21782054493256212, integrate(0.1, 1), eps);
        EXPECT_EQ(516, f.exchange_count());
        // Results are numerically unstable
        EXPECT_SOFT_NEAR(0, integrate(0.01, 0.1), 0.01);
        EXPECT_EQ(1048577, f.exchange_count());
    }
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace celeritas
