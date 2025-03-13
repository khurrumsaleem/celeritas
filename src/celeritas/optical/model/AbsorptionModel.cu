//---------------------------------*-CUDA-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/model/AbsorptionModel.cu
//---------------------------------------------------------------------------//
#include "AbsorptionModel.hh"

#include "celeritas/optical/action/ActionLauncher.device.hh"
#include "celeritas/optical/action/TrackSlotExecutor.hh"

#include "AbsorptionExecutor.hh"
#include "../CoreParams.hh"
#include "../CoreState.hh"
#include "../InteractionApplier.hh"

namespace celeritas
{
namespace optical
{
//---------------------------------------------------------------------------//
/*!
 * Interact with device data.
 */
void AbsorptionModel::step(CoreParams const& params,
                           CoreStateDevice& state) const
{
    auto execute = make_action_thread_executor(
        params.ptr<MemSpace::native>(),
        state.ptr(),
        this->action_id(),
        InteractionApplier{AbsorptionExecutor{}});
    static ActionLauncher<decltype(execute)> const launch_kernel(*this);
    launch_kernel(state, execute);
}

//---------------------------------------------------------------------------//
}  // namespace optical
}  // namespace celeritas
