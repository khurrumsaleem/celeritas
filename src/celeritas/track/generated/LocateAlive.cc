//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/track/generated/LocateAlive.cc
//! \note Auto-generated by gen-trackinit.py: DO NOT MODIFY!
//---------------------------------------------------------------------------//
#include <utility>

#include "corecel/sys/MultiExceptionHandler.hh"
#include "corecel/sys/ThreadId.hh"
#include "corecel/Types.hh"
#include "celeritas/track/detail/LocateAliveLauncher.hh" // IWYU pragma: associated

namespace celeritas
{
namespace generated
{
void locate_alive(
    CoreHostRef const& core_data)
{
    MultiExceptionHandler capture_exception;
    detail::LocateAliveLauncher<MemSpace::host> launch(core_data);
    #pragma omp parallel for
    for (ThreadId::size_type i = 0; i < core_data.states.size(); ++i)
    {
        CELER_TRY_HANDLE(launch(ThreadId{i}), capture_exception);
    }
    log_and_rethrow(std::move(capture_exception));
}

}  // namespace generated
}  // namespace celeritas
