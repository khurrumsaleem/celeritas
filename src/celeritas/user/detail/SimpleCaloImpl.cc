//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/user/detail/SimpleCaloImpl.cc
//---------------------------------------------------------------------------//
#include "SimpleCaloImpl.hh"

#include "corecel/Config.hh"

#include "corecel/Types.hh"
#include "corecel/sys/MultiExceptionHandler.hh"
#include "corecel/sys/ThreadId.hh"

#include "SimpleCaloExecutor.hh"  // IWYU pragma: associated

namespace celeritas
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Accumulate energy deposition on host.
 */
void simple_calo_accum(HostRef<StepStateData> const& step,
                       HostRef<SimpleCaloStateData>& calo)
{
    CELER_EXPECT(step && calo);
    MultiExceptionHandler capture_exception;
    SimpleCaloExecutor execute{step, calo};
    size_type const size = step.size();
#if CELERITAS_OPENMP == CELERITAS_OPENMP_TRACK
#    pragma omp parallel for
#endif
    for (ThreadId::size_type i = 0; i < size; ++i)
    {
        CELER_TRY_HANDLE(execute(ThreadId{i}), capture_exception);
    }
    log_and_rethrow(std::move(capture_exception));
}

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
