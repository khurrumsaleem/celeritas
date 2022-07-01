//---------------------------------*-CUDA-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/phys/generated/PreStepAction.cu
//! \note Auto-generated by gen-action.py: DO NOT MODIFY!
//---------------------------------------------------------------------------//
#include "PreStepAction.hh"

#include "corecel/device_runtime_api.h"
#include "corecel/Assert.hh"
#include "corecel/Types.hh"
#include "corecel/sys/KernelParamCalculator.device.hh"
#include "corecel/sys/Device.hh"
#include "celeritas/global/TrackLauncher.hh"
#include "../detail/PreStepActionImpl.hh"

namespace celeritas
{
namespace generated
{
namespace
{
__global__ void
#if CELERITAS_LAUNCH_BOUNDS
#if CELERITAS_USE_CUDA && (__CUDA_ARCH__ == 700) // Tesla V100-SXM2-16GB
__launch_bounds__(1024, 4)
#endif
#if CELERITAS_USE_HIP && defined(__gfx90a__)
__launch_bounds__(1024, 7)
#endif
#endif // CELERITAS_LAUNCH_BOUNDS
pre_step_kernel(CoreDeviceRef const data
)
{
    auto tid = KernelParamCalculator::thread_id();
    if (!(tid < data.states.size()))
        return;

    auto launch = make_track_launcher(data, detail::pre_step_track);
    launch(tid);
}
} // namespace

void PreStepAction::execute(const CoreDeviceRef& data) const
{
    CELER_EXPECT(data);
    CELER_LAUNCH_KERNEL(pre_step,
                        celeritas::device().default_block_size(),
                        data.states.size(),
                        data);
}

} // namespace generated
} // namespace celeritas