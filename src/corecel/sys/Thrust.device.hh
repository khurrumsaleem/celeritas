//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file corecel/sys/Thrust.device.hh
//! \brief Platform and version-specific thrust setup
//---------------------------------------------------------------------------//
#pragma once

#include <thrust/execution_policy.h>
#include <thrust/mr/allocator.h>
#include <thrust/version.h>

#include "corecel/Config.hh"

#include "corecel/Assert.hh"

#include "Device.hh"
#include "Stream.hh"
#include "ThreadId.hh"

namespace celeritas
{
#if CELERITAS_USE_CUDA
namespace thrust_native = thrust::cuda;
#elif CELERITAS_USE_HIP
namespace thrust_native = thrust::hip;
#endif

//---------------------------------------------------------------------------//
// ENUMERATIONS
//---------------------------------------------------------------------------//
//! Execution semantics for thrust algorithms.
enum class ThrustExecMode
{
    Sync,
    Async,
};

//---------------------------------------------------------------------------//
// FREE FUNCTIONS
//---------------------------------------------------------------------------//
/*!
 * Returns an execution policy depending on thrust's version.
 */
template<ThrustExecMode T = ThrustExecMode::Async>
inline auto& thrust_execution_policy()
{
    if constexpr (T == ThrustExecMode::Async)
    {
#if THRUST_MAJOR_VERSION == 1 && THRUST_MINOR_VERSION < 16
        return thrust_native::par;
#else
        return thrust_native::par_nosync;
#endif
    }
    else
    {
        return thrust_native::par;
    }
#if (__CUDACC_VER_MAJOR__ < 11) \
    || (__CUDACC_VER_MAJOR__ == 11 && __CUDACC_VER_MINOR__ < 5)
    CELER_ASSERT_UNREACHABLE();
#endif
}

//---------------------------------------------------------------------------//
/*!
 * Returns an execution space for the given stream.
 *
 * The template parameter defines whether the algorithm should be executed
 * synchronously or asynchrounously.
 */
template<ThrustExecMode T = ThrustExecMode::Async>
inline auto thrust_execute_on(StreamId stream_id)
{
    if constexpr (T == ThrustExecMode::Sync)
    {
        return thrust_execution_policy<T>().on(
            celeritas::device().stream(stream_id).get());
    }
    else if constexpr (T == ThrustExecMode::Async)
    {
        using Alloc = thrust::mr::allocator<char, Stream::ResourceT>;
        Stream& stream = celeritas::device().stream(stream_id);
        return thrust_execution_policy<T>()(Alloc(&stream.memory_resource()))
            .on(stream.get());
    }
#if (__CUDACC_VER_MAJOR__ < 11) \
    || (__CUDACC_VER_MAJOR__ == 11 && __CUDACC_VER_MINOR__ < 5)
    CELER_ASSERT_UNREACHABLE();
#endif
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
