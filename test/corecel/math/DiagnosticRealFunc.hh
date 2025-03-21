//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file corecel/math/DiagnosticRealFunc.hh
//---------------------------------------------------------------------------//
#pragma once

#include <cmath>
#include <utility>

#include "corecel/Assert.hh"
#include "corecel/Types.hh"
#include "corecel/io/Logger.hh"

namespace celeritas
{
namespace test
{
//---------------------------------------------------------------------------//
/*!
 * Wrap a numerical single-argument function with a counter and logger.
 *
 * This takes a function:
 * \f[ f\colon\mathbb{R^1}\to\mathbb{R^1} \f]
 *
 * and adds a counter that increments every time the function is invoked. This
 * is very useful for unit testing the convergence properties of numerical
 * integrators, root finders, etc.
 *
 * Example: \code
    DiagnosticFunc f{[](real_type x) { return 2 * x; }};
    Integrator integrate{f};
    EXPECT_SOFT_EQ(4 - 1, integrate(1, 2));
    EXPECT_EQ(3, f.exchange_count());
 \endcode
 *
 * This wrapper also checks the input and output for NaN, and it outputs the
 * function counter and evaluation to the logger (export `CELER_LOG=debug`).
 */
template<class F>
class DiagnosticRealFunc
{
  public:
    //! Construct by forwarding a function
    explicit DiagnosticRealFunc(F&& eval) : eval_{std::forward<F>(eval)} {}

    //! Get and reset the counter
    size_type exchange_count() { return std::exchange(count_, 0); }

    // Evaluate the underlying function and increment the counter
    real_type operator()(real_type v);

  private:
    F eval_;
    size_type count_{0};
};

//---------------------------------------------------------------------------//
/*!
 * Check the first value for double precision or second for single.
 *
 * This is useful for comparing the \c exchange_count result.
 */
template<class T>
inline T if_double_else(T a, T b)
{
    if constexpr (CELERITAS_REAL_TYPE == CELERITAS_REAL_TYPE_DOUBLE)
    {
        CELER_DISCARD(b);  // Need for old compilers
        return a;
    }
    else
    {
        CELER_DISCARD(a);  // Need for old compilers
        return b;
    }
}

//---------------------------------------------------------------------------//
// TEMPLATE DEDUCTION
//---------------------------------------------------------------------------//

template<class F, class... Args>
DiagnosticRealFunc(F&&, Args...) -> DiagnosticRealFunc<F>;

//---------------------------------------------------------------------------//
// INLINE FUNCTIONS
//---------------------------------------------------------------------------//
/*!
 * Evaluate the underlying function and increment the counter.
 */
template<class F>
real_type DiagnosticRealFunc<F>::operator()(real_type v)
{
    // Always-on debug check for NaN input
    if (CELER_UNLIKELY(std::isnan(v)))
    {
        CELER_DEBUG_FAIL("nan input given to function", precondition);
    }

    // Increment the counter and evaluate the function
    ++count_;
    auto result = eval_(v);
    static_assert(std::is_same_v<decltype(result), real_type>,
                  "function should return real_type");

    // Log the result
    if (count_ < 20)
    {
        CELER_LOG(debug) << count_ << ": f(" << v << ") -> " << result;
    }
    else if (count_ == 20)
    {
        CELER_LOG(debug) << "Suppressing further log messages";
    }

    // Always-on debug check for NaN output
    if (CELER_UNLIKELY(std::isnan(result)))
    {
        CELER_DEBUG_FAIL("nan output returned from function", postcondition);
    }
    return result;
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace celeritas
