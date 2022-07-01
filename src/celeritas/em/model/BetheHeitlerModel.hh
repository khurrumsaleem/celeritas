//----------------------------------*-C++-*----------------------------------//
// Copyright 2020-2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/em/model/BetheHeitlerModel.hh
//---------------------------------------------------------------------------//
#pragma once

#include "celeritas/em/data/BetheHeitlerData.hh"
#include "celeritas/phys/ImportedModelAdapter.hh"
#include "celeritas/phys/Model.hh"
#include "celeritas/phys/ParticleParams.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Set up and launch the Bethe-Heitler model interaction.
 */
class BetheHeitlerModel final : public Model
{
  public:
    //!@{
    //! Type aliases
    using SPConstImported = std::shared_ptr<const ImportedProcesses>;
    //!@}

  public:
    // Construct from model ID and other necessary data
    BetheHeitlerModel(ActionId              id,
                      const ParticleParams& particles,
                      SPConstImported       data,
                      bool                  enable_lpm);

    // Particle types and energy ranges that this model applies to
    SetApplicability applicability() const final;

    // Get the microscopic cross sections for the given particle and material
    MicroXsBuilders micro_xs(Applicability) const final;

    // Apply the interaction kernel on host
    void execute(CoreHostRef const&) const final;

    // Apply the interaction kernel on device
    void execute(CoreDeviceRef const&) const final;

    // ID of the model
    ActionId action_id() const final;

    //! Short name for the interaction kernel
    std::string label() const final { return "conv-bethe-heitler"; }

    //! Name of the model, for user interaction
    std::string description() const final
    {
        return "Bethe-Heitler gamma conversion";
    }

  private:
    BetheHeitlerData     interface_;
    ImportedModelAdapter imported_;
};

//---------------------------------------------------------------------------//
} // namespace celeritas