//------------------------------ -*- cuda -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file geocel/vg/detail/VecgeomSetup.cu
//---------------------------------------------------------------------------//
#include "VecgeomSetup.hh"

#include <VecGeom/management/BVHManager.h>

#include "corecel/data/DeviceVector.hh"

#ifdef VECGEOM_USE_SURF
#    include <VecGeom/surfaces/cuda/BrepCudaManager.h>
#endif

#include "corecel/Assert.hh"
#include "corecel/Macros.hh"
#include "corecel/sys/KernelLauncher.device.hh"
#include "corecel/sys/ThreadId.hh"

#ifdef VECGEOM_USE_SURF
using BrepCudaManager = vgbrep::BrepCudaManager<vecgeom::Precision>;
using SurfData = vgbrep::SurfData<vecgeom::Precision>;
#endif

namespace celeritas
{
namespace detail
{
namespace
{
//---------------------------------------------------------------------------//
//! Access
struct BvhGetter
{
    CudaBVH_t const** dest{nullptr};

    CELER_FUNCTION void operator()(ThreadId tid)
    {
        CELER_EXPECT(tid == ThreadId{0});
        *dest = vecgeom::cuda::BVHManager::GetBVH(0);
    }
};
}  // namespace

//---------------------------------------------------------------------------//
/*!
 * Get pointers to the device BVH after setup, for consistency checking.
 */
CudaPointers<CudaBVH_t const> bvh_pointers_device()
{
    CudaPointers<CudaBVH_t const> result;

    // Copy from kernel using 1-thread launch
    {
        DeviceVector<CudaBVH_t const*> bvh_ptr{1, StreamId{}};
        BvhGetter execute_thread{bvh_ptr.data()};
        static KernelLauncher<decltype(execute_thread)> const launch_kernel(
            "vecgeom-get-bvhptr");
        launch_kernel(1u, StreamId{}, execute_thread);
        CELER_DEVICE_API_CALL(DeviceSynchronize());
        bvh_ptr.copy_to_host({&result.kernel, 1});
    }

    // Copy from symbol using runtime API
    CELER_DEVICE_API_CALL(
        MemcpyFromSymbol(&result.symbol,
#if VECGEOM_VERSION >= VECGEOM_V2
                         vecgeom::cuda::dBVH<BvhPrecision>,
                         sizeof(vecgeom::cuda::dBVH<BvhPrecision>),
#else
                         vecgeom::cuda::dBVH,
                         sizeof(vecgeom::cuda::dBVH),
#endif
                         0,
                         CELER_DEVICE_API_SYMBOL(MemcpyDeviceToHost)));
    CELER_DEVICE_API_CALL(DeviceSynchronize());

    return result;
}

//---------------------------------------------------------------------------//
// VECGEOM SURFACE
//---------------------------------------------------------------------------//
#ifdef VECGEOM_USE_SURF
void setup_surface_tracking_device(SurfData const& surf_data)
{
    BrepCudaManager::Instance().TransferSurfData(surf_data);
    CELER_DEVICE_API_CALL(DeviceSynchronize());
}

void teardown_surface_tracking_device()
{
    BrepCudaManager::Instance().Cleanup();
}
#endif

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
