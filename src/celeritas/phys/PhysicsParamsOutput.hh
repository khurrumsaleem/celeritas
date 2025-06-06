//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/phys/PhysicsParamsOutput.hh
//---------------------------------------------------------------------------//
#pragma once
#include <memory>

#include "corecel/io/OutputInterface.hh"

namespace celeritas
{
class PhysicsParams;
//---------------------------------------------------------------------------//
/*!
 * Save detailed debugging information about the physics in use.
 */
class PhysicsParamsOutput final : public OutputInterface
{
  public:
    //!@{
    //! \name Type aliases
    using SPConstPhysicsParams = std::shared_ptr<PhysicsParams const>;
    //!@}

  public:
    // Construct from shared physics data
    explicit PhysicsParamsOutput(SPConstPhysicsParams physics);

    //! Category of data to write
    Category category() const final { return Category::internal; }

    //! Name of the entry inside the category.
    std::string_view label() const final { return "physics"; }

    // Write output to the given JSON object
    void output(JsonPimpl*) const final;

  private:
    SPConstPhysicsParams physics_;
};

//---------------------------------------------------------------------------//
}  // namespace celeritas
