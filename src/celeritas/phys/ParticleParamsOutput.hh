//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/phys/ParticleParamsOutput.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>

#include "corecel/io/OutputInterface.hh"

namespace celeritas
{
class ParticleParams;
//---------------------------------------------------------------------------//
/*!
 * Save detailed debugging information about particles in use.
 */
class ParticleParamsOutput final : public OutputInterface
{
  public:
    //!@{
    //! \name Type aliases
    using SPConstParticleParams = std::shared_ptr<ParticleParams const>;
    //!@}

  public:
    // Construct from shared physics data
    explicit ParticleParamsOutput(SPConstParticleParams particles);

    //! Category of data to write
    Category category() const final { return Category::internal; }

    //! Name of the entry inside the category.
    std::string_view label() const final { return "particle"; }

    // Write output to the given JSON object
    void output(JsonPimpl*) const final;

  private:
    SPConstParticleParams particles_;
};

//---------------------------------------------------------------------------//
}  // namespace celeritas
