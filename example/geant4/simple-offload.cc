//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file geant4/simple-offload.cc
//---------------------------------------------------------------------------//

#include <algorithm>
#include <iterator>
#include <type_traits>

// Geant4
#include <FTFP_BERT.hh>
#include <G4Box.hh>
#include <G4Electron.hh>
#include <G4Gamma.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <G4Positron.hh>
#include <G4SystemOfUnits.hh>
#include <G4Threading.hh>
#include <G4ThreeVector.hh>
#include <G4Track.hh>
#include <G4TrackStatus.hh>
#include <G4Types.hh>
#include <G4UserEventAction.hh>
#include <G4UserRunAction.hh>
#include <G4UserTrackingAction.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4Version.hh>
#if G4VERSION_NUMBER >= 1100
#    include <G4RunManagerFactory.hh>
#else
#    include <G4MTRunManager.hh>
#endif

// Celeritas
#include <CeleritasG4.hh>

// Utility includes
#include <corecel/io/Logger.hh>

using celeritas::UserActionIntegration;

namespace
{
//---------------------------------------------------------------------------//
class DetectorConstruction final : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction()
        : aluminum_{new G4Material{
              "Aluminium", 13., 26.98 * g / mole, 2.700 * g / cm3}}
    {
    }

    G4VPhysicalVolume* Construct() final
    {
        CELER_LOG_LOCAL(status) << "Setting up geometry";
        auto* box = new G4Box("world", 100 * cm, 100 * cm, 100 * cm);
        auto* lv = new G4LogicalVolume(box, aluminum_, "world");
        auto* pv = new G4PVPlacement(
            0, G4ThreeVector{}, lv, "world", nullptr, false, 0);
        return pv;
    }

    void ConstructSDandField() final {}

  private:
    G4Material* aluminum_;
};

//---------------------------------------------------------------------------//
// Generate 200 MeV pi+
class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction()
    {
        auto g4particle_def
            = G4ParticleTable::GetParticleTable()->FindParticle(211);
        gun_.SetParticleDefinition(g4particle_def);
        gun_.SetParticleEnergy(200 * MeV);
        gun_.SetParticlePosition(G4ThreeVector{0, 0, 0});  // origin
        gun_.SetParticleMomentumDirection(G4ThreeVector{1, 0, 0});  // +x
    }

    void GeneratePrimaries(G4Event* event) final
    {
        CELER_LOG_LOCAL(status) << "Generating primaries";
        gun_.GeneratePrimaryVertex(event);
    }

  private:
    G4ParticleGun gun_;
};

//---------------------------------------------------------------------------//
class RunAction final : public G4UserRunAction
{
  public:
    void BeginOfRunAction(G4Run const* run) final
    {
        auto& uai = UserActionIntegration::Instance();
        uai.BeginOfRunAction(run);

        // Demonstrate offload mode query
        using Mode = celeritas::OffloadMode;
        auto msg = CELER_LOG(info);
        msg << "Celeritas is ";
        switch (uai.GetMode())
        {
            case Mode::disabled:
                msg << "disabled: only Geant4 is tracking";
                break;
            case Mode::kill_offload:
                msg << "killing EM tracks";
                break;
            case Mode::enabled:
                msg << "active: EM tracks are sent from Geant4";
                break;
            default:
                msg << "misbehaving, mode is unexpected!";
        }
    }
    void EndOfRunAction(G4Run const* run) final
    {
        UserActionIntegration::Instance().EndOfRunAction(run);
    }
};

//---------------------------------------------------------------------------//
class EventAction final : public G4UserEventAction
{
  public:
    void BeginOfEventAction(G4Event const* event) final
    {
        UserActionIntegration::Instance().BeginOfEventAction(event);
    }

    void EndOfEventAction(G4Event const* event) final
    {
        UserActionIntegration::Instance().EndOfEventAction(event);
    }
};

//---------------------------------------------------------------------------//
class TrackingAction final : public G4UserTrackingAction
{
    void PreUserTrackingAction(G4Track const* track) final
    {
        UserActionIntegration::Instance().PreUserTrackingAction(
            const_cast<G4Track*>(track));
    }
};

//---------------------------------------------------------------------------//
class ActionInitialization final : public G4VUserActionInitialization
{
  public:
    void BuildForMaster() const final { this->SetUserAction(new RunAction{}); }
    void Build() const final
    {
        this->SetUserAction(new PrimaryGeneratorAction{});
        this->SetUserAction(new RunAction{});
        this->SetUserAction(new EventAction{});
        this->SetUserAction(new TrackingAction{});
    }
};

//---------------------------------------------------------------------------//
/*!
 * Construct options for Celeritas.
 */
celeritas::SetupOptions MakeOptions()
{
    celeritas::SetupOptions opts;

    opts.make_along_step = celeritas::UniformAlongStepFactory();

    // NOTE: since no SD is enabled, we must manually disable Celeritas hit
    // processing
    opts.sd.enabled = false;

    // Celeritas does not support EmStandard MSC physics above 200 MeV
    opts.ignore_processes = {"CoulombScat"};

    // Save GDML file
    if (G4VERSION_NUMBER >= 1070)
    {
        opts.geometry_output_file = "simple-offload.gdml";
    }
    else
    {
        CELER_LOG(info) << "Not setting simple offload: older versions of "
                           "Geant4 may fail on CI due to files stepping on "
                           "each other";
    }

    opts.output_file = "simple-offload.out.json";
    return opts;
}

//---------------------------------------------------------------------------//
}  // namespace

int main()
{
    auto run_manager = [] {
#if G4VERSION_NUMBER >= 1100
        return std::unique_ptr<G4RunManager>{
            G4RunManagerFactory::CreateRunManager()};
#else
        return std::make_unique<G4RunManager>();
#endif
    }();

    run_manager->SetUserInitialization(new DetectorConstruction{});
    run_manager->SetUserInitialization(new FTFP_BERT{/* verbosity = */ 0});
    run_manager->SetUserInitialization(new ActionInitialization());

    UserActionIntegration::Instance().SetOptions(MakeOptions());

    run_manager->Initialize();
    run_manager->BeamOn(2);

    return 0;
}
