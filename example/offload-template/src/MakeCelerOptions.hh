//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file offload-template/src/MakeCelerOptions.hh
//---------------------------------------------------------------------------//
#pragma once

#include <accel/SetupOptions.hh>

namespace celeritas
{
namespace example
{
// Build options to set up Celeritas
celeritas::SetupOptions MakeCelerOptions();

}  // namespace example
}  // namespace celeritas
