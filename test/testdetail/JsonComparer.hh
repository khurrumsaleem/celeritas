//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file testdetail/JsonComparer.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <gtest/gtest.h>

#include "corecel/Config.hh"

#include "corecel/math/SoftEqual.hh"

namespace celeritas
{
namespace testdetail
{
//---------------------------------------------------------------------------//
/*!
 * Perform an equality test (or soft equality) on two JSON objects.
 */
class JsonComparer
{
  public:
    using result_type = ::testing::AssertionResult;

  public:
    //! Construct with optional tolerance
    template<class... C>
    JsonComparer(C&&... args) : compare_{std::forward<C>(args)...}
    {
    }

    // Compare two strings for equality
    result_type operator()(std::string_view expected, std::string_view actual);

    // Compare the same strings for equality (testing)
    result_type operator()(std::string_view expected)
    {
        return (*this)(expected, expected);
    }

  private:
    struct Failure
    {
        std::string where;
        std::string what;
        std::string expected;
        std::string actual;
    };

    using Compare = EqualOr<SoftEqual<real_type>>;
    using VecFailure = std::vector<Failure>;

    Compare compare_;
    struct Impl;
};

//---------------------------------------------------------------------------//
}  // namespace testdetail
}  // namespace celeritas
