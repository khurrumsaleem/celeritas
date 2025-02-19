//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/setup/StandaloneInput.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <vector>

#include "celeritas/phys/Primary.hh"

namespace celeritas
{
class CoreParams;

namespace inp
{
struct StandaloneInput;
}
namespace setup
{
//---------------------------------------------------------------------------//
//! Result from loaded standalone input to be used in front-end apps
struct StandaloneLoaded
{
    using VecPrimary = std::vector<Primary>;
    using VecEvent = std::vector<VecPrimary>;

    //! Problem setup
    std::shared_ptr<CoreParams> core_params;
    //! Events to be run
    VecEvent events;
};

//---------------------------------------------------------------------------//
// Completely set up a Celeritas problem from a standalone input
StandaloneLoaded standalone_input(inp::StandaloneInput& si);

//---------------------------------------------------------------------------//
}  // namespace setup
}  // namespace celeritas
