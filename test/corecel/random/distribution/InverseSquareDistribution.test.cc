//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file corecel/random/distribution/InverseSquareDistribution.test.cc
//---------------------------------------------------------------------------//
#include "corecel/random/distribution/InverseSquareDistribution.hh"

#include "corecel/random/Histogram.hh"

#include "celeritas_test.hh"

namespace celeritas
{
namespace test
{
//---------------------------------------------------------------------------//
TEST(InverseSquareDistributionTest, bin)
{
    int num_samples = 9000;

    double min = 0.1;
    double max = 0.9;

    InverseSquareDistribution<double> sample_esq{min, max};
    std::mt19937 rng;

    Histogram histogram(10, {0, 10});
    for ([[maybe_unused]] int i : range(num_samples))
    {
        histogram(1.0 / sample_esq(rng));
    }

    static unsigned int const expected_counts[]
        = {0, 944, 1043, 959, 972, 1027, 1045, 981, 1009, 1020};
    EXPECT_VEC_EQ(expected_counts, histogram.counts());
    EXPECT_GE(histogram.min(), min);
    EXPECT_LE(histogram.max(), max);
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace celeritas
