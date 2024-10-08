//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/CerenkovParams.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Types.hh"
#include "corecel/data/CollectionMirror.hh"
#include "corecel/data/ParamsDataInterface.hh"

#include "CerenkovData.hh"

namespace celeritas
{
namespace optical
{
class MaterialParams;

//---------------------------------------------------------------------------//
/*!
 * Build and manage Cerenkov data.
 */
class CerenkovParams final : public ParamsDataInterface<CerenkovData>
{
  public:
    //!@{
    //! \name Type aliases
    using SPConstMaterial = std::shared_ptr<MaterialParams const>;
    //!@}

  public:
    // Construct with optical property data
    explicit CerenkovParams(SPConstMaterial material);

    //! Access physics material on the host
    HostRef const& host_ref() const final { return data_.host_ref(); }

    //! Access physics material on the device
    DeviceRef const& device_ref() const final { return data_.device_ref(); }

  private:
    CollectionMirror<CerenkovData> data_;
};

//---------------------------------------------------------------------------//
}  // namespace optical
}  // namespace celeritas
