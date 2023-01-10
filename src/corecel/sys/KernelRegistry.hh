//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file corecel/sys/KernelRegistry.hh
//---------------------------------------------------------------------------//
#pragma once

#include <atomic>
#include <iosfwd>  // IWYU pragma: keep
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "corecel/Assert.hh"
#include "corecel/OpaqueId.hh"

#include "KernelAttributes.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
struct KernelProfiling
{
    //!< Number of times launched
    std::atomic<int> num_launches{0};
    //!< Number of threads integrated over all launches
    std::atomic<long long> accum_threads{0};

    // Increment atomic counters given the number of threads
    inline void log_launch(int num_threads);
};

//---------------------------------------------------------------------------//
struct KernelMetadata
{
    std::string name;
    KernelAttributes attributes;
    KernelProfiling profiling;
};

//! Ordered identifiers for registered kernels
using KernelId = OpaqueId<KernelMetadata>;

//---------------------------------------------------------------------------//
/*!
 * Keep track of kernels and launches.
 *
 * Every "insert" creates a unique \c KernelMetadata entry in a thread-safe
 * fashion (in case multiple threads are launching kernels for the first time).
 * Thus every kernel added to the registry needs a \c static local data (i.e.,
 * \c KernelParamCalculator) to track whether the kernel has been added and to
 * keep a reference to the returned profiling data counter. Kernels are always
 * added sequentially and can never be removed from the registry once added.
 * Kernels that share the same name will create independent entries!
 *
 * This class has a thread-safe methods because it's meant to be shared
 * across multiple threads when running. Generally \c insert is the only method
 * expected to have contention across threads.
 */
class KernelRegistry
{
  public:
    // Whether profiling metrics (launch count, max threads) are collected
    static bool profiling();

    // Construct without any data
    KernelRegistry() = default;

    //// CONSTRUCTION ////

    // Register a kernel and return optional reference to profiling info
    KernelProfiling* insert(char const* name, KernelAttributes&& attrs);

    //// ACCESSORS ////

    // Number of kernel diagnostics available
    KernelId::size_type num_kernels() const;

    // Access kernel data for a single kernel
    KernelMetadata const& kernel(KernelId id) const;

  private:
    using UPKM = std::unique_ptr<KernelMetadata>;

    mutable std::mutex kernels_mutex_;
    std::vector<UPKM> kernels_;
};

//---------------------------------------------------------------------------//
// FREE FUNCTIONS
//---------------------------------------------------------------------------//
// Globally shared registry of kernels for end-of-program diagnostics
KernelRegistry& kernel_registry();

// Write kernel statistics to a stream
std::ostream& operator<<(std::ostream& os, KernelMetadata const& md);

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Accumulate counters for a kernel launch.
 */
void KernelProfiling::log_launch(int num_threads)
{
    CELER_EXPECT(num_threads > 0);

    // Increment launches by 1 and thread count by num_threads.
    // We don't care in what order these values are written.
    this->num_launches.fetch_add(1, std::memory_order_relaxed);
    this->accum_threads.fetch_add(num_threads, std::memory_order_relaxed);
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
