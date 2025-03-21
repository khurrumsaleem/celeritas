//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/ext/detail/MuHadEmStandardPhysics.hh
//---------------------------------------------------------------------------//
#pragma once

#include <G4VPhysicsConstructor.hh>

namespace celeritas
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Construct EM standard physics not implemented in Celeritas.
 *
 * \todo Remove muon physics from this constructor once it is fully supported
 * in Celeritas.
 */
class MuHadEmStandardPhysics : public G4VPhysicsConstructor
{
  public:
    // Set up during construction
    explicit MuHadEmStandardPhysics(int verbosity);

    // Set up minimal EM particle list
    void ConstructParticle() override;
    // Set up process list
    void ConstructProcess() override;

  private:
    void construct_particle();
    void construct_process();
};

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
