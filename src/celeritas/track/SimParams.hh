//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/track/SimParams.hh
//---------------------------------------------------------------------------//
#pragma once

#include <unordered_map>

#include "corecel/Types.hh"
#include "corecel/data/CollectionMirror.hh"
#include "corecel/data/ParamsDataInterface.hh"
#include "celeritas/phys/PDGNumber.hh"

#include "SimData.hh"

namespace celeritas
{
class ParticleParams;
struct ImportData;

//---------------------------------------------------------------------------//
/*!
 * Manage persistent simulation data.
 */
class SimParams final : public ParamsDataInterface<SimParamsData>
{
  public:
    //!@{
    //! \name Type aliases
    using SPConstParticles = std::shared_ptr<ParticleParams const>;
    //!@}

    //! Input data to construct this class
    struct Input
    {
        SPConstParticles particles;
        std::unordered_map<PDGNumber, LoopingThreshold> looping;
    };

  public:
    // Construct with imported data and default max field substeps
    static std::shared_ptr<SimParams>
    from_import(ImportData const&, SPConstParticles);

    // Construct with imported data
    static std::shared_ptr<SimParams> from_import(ImportData const&,
                                                  SPConstParticles,
                                                  short int max_field_substeps);

    // Construct with simulation input data
    explicit SimParams(Input const&);

    // Construct without loop counters (for neutral-only simulations)
    SimParams();

    //! Access data on host
    HostRef const& host_ref() const final { return data_.host_ref(); }

    //! Access data on device
    DeviceRef const& device_ref() const final { return data_.device_ref(); }

  private:
    // Host/device storage and reference
    CollectionMirror<SimParamsData> data_;
};

//---------------------------------------------------------------------------//
}  // namespace celeritas
