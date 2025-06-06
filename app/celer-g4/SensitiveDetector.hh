//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-g4/SensitiveDetector.hh
//---------------------------------------------------------------------------//
#pragma once

#include <G4THitsCollection.hh>
#include <G4VSensitiveDetector.hh>

#include "celeritas/Types.hh"

#include "SensitiveHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

namespace celeritas
{
namespace app
{
//---------------------------------------------------------------------------//
/*!
 * Example sensitive detector.
 */
class SensitiveDetector final : public G4VSensitiveDetector
{
    //!@{
    //! \name Type aliases
    using SensitiveHitsCollection = G4THitsCollection<SensitiveHit>;
    //!@}

  public:
    explicit SensitiveDetector(std::string name);

  protected:
    void Initialize(G4HCofThisEvent*) final;
    bool ProcessHits(G4Step*, G4TouchableHistory*) final;

  private:
    int hcid_{-1};
    SensitiveHitsCollection* collection_{nullptr};
};

//---------------------------------------------------------------------------//
}  // namespace app
}  // namespace celeritas
