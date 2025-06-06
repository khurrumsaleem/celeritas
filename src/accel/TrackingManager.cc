//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file accel/TrackingManager.cc
//---------------------------------------------------------------------------//
#include "TrackingManager.hh"

#include <G4ProcessManager.hh>
#include <G4ProcessVector.hh>
#include <G4Track.hh>

#include "corecel/Assert.hh"
#include "corecel/cont/Range.hh"

#include "ExceptionConverter.hh"
#include "LocalTransporter.hh"
#include "SharedParams.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Construct a tracking manager with data needed to offload to Celeritas.
 *
 * \note The shared/local pointers must remain valid for the lifetime of the
 * run.
 */
TrackingManager::TrackingManager(SharedParams const* params,
                                 LocalTransporter* local)
    : params_(params), transport_(local)
{
    CELER_EXPECT(params_);
    CELER_EXPECT(transport_);
}

//---------------------------------------------------------------------------//
/*!
 * Build physics tables for this particle.
 *
 * Messaged by the \c G4ParticleDefinition who stores us whenever cross-section
 * tables have to be rebuilt (i.e. if new materials have been defined). An
 * override is needed for Celeritas as it uses the particle's process manager
 * and tables to initialize its own physics data for the particle, and this is
 * disabled when a custom tracking manager is used. Note that this also means
 * we could have filters in HandOverOneTrack to hand back the track to the
 * general G4TrackingManager if matching a predicate(s).
 *
 * The implementation follows that in \c G4VUserPhysicsList::BuildPhysicsTable
 * , see also Geant4 Extended Example runAndEvent/RE07.
 */
void TrackingManager::BuildPhysicsTable(G4ParticleDefinition const& part)
{
    CELER_LOG(debug) << "Building physics table for " << part.GetParticleName();

    CELER_VALIDATE(params_->mode() != SharedParams::Mode::disabled,
                   << "Celeritas tracking manager cannot be active when "
                      "Celeritas is disabled");
    G4ProcessManager* pManagerShadow = part.GetMasterProcessManager();
    G4ProcessManager* pManager = part.GetProcessManager();
    CELER_ASSERT(pManager);

    G4ProcessVector* pVector = pManager->GetProcessList();
    CELER_ASSERT(pVector);
    for (auto j : range(pVector->size()))
    {
        G4VProcess* proc = (*pVector)[j];
        if (pManagerShadow == pManager)
        {
            proc->BuildPhysicsTable(part);
        }
        else
        {
            proc->BuildWorkerPhysicsTable(part);
        }
    }
}

//---------------------------------------------------------------------------//
/*!
 * Prepare physics tables for this particle.
 *
 * Messaged by the \c G4ParticleDefinition who stores us whenever cross-section
 * tables have to be rebuilt (i.e. if new materials have been defined). As with
 * BuildPhysicsTable, we override this to ensure all Geant4
 * process/cross-section data is available for Celeritas to use.
 *
 * The implementation follows that in \c
 * G4VUserPhysicsList::PreparePhysicsTable , see also Geant4 Extended Example
 * runAndEvent/RE07.
 */
void TrackingManager::PreparePhysicsTable(G4ParticleDefinition const& part)
{
    CELER_LOG(debug) << "Preparing physics table for "
                     << part.GetParticleName();

    G4ProcessManager* pManagerShadow = part.GetMasterProcessManager();
    G4ProcessManager* pManager = part.GetProcessManager();
    CELER_ASSERT(pManager);

    G4ProcessVector* pVector = pManager->GetProcessList();
    CELER_ASSERT(pVector);
    for (auto j : range(pVector->size()))
    {
        G4VProcess* proc = (*pVector)[j];
        if (pManagerShadow == pManager)
        {
            proc->PreparePhysicsTable(part);
        }
        else
        {
            proc->PrepareWorkerPhysicsTable(part);
        }
    }
}

//---------------------------------------------------------------------------//
/*!
 * Offload the incoming track to Celeritas.
 */
void TrackingManager::HandOverOneTrack(G4Track* track)
{
    CELER_EXPECT(track);

    if (CELER_UNLIKELY(!validated_))
    {
        CELER_TRY_HANDLE(
            CELER_VALIDATE(
                params_->mode()
                    == (*transport_ ? SharedParams::Mode::enabled
                                    : SharedParams::Mode::kill_offload),
                << "Celeritas was not initialized properly (maybe "
                   "BeginOfRunAction was not called?)"),
            ExceptionConverter("celer.track.validate"));
        validated_ = true;
    }

    if (*transport_)
    {
        // Offload this track to Celeritas for transport
        CELER_TRY_HANDLE(transport_->Push(*track),
                         ExceptionConverter("celer.track.push", params_));
    }

    // G4VTrackingManager takes ownership, so kill Geant4 track
    track->SetTrackStatus(fStopAndKill);
    delete track;
}

//---------------------------------------------------------------------------//
/*!
 * Complete processing of any buffered tracks.
 *
 * Note that this is called in \c G4EventManager::DoProcessing(G4Event*) after
 * the after the main tracking loop has completed.
 *
 * That is done to allow for models that may add "onload" particles back to
 * Geant4.
 */
void TrackingManager::FlushEvent()
{
    if (*transport_)
    {
        CELER_TRY_HANDLE(transport_->Flush(),
                         ExceptionConverter("celer.event.flush", params_));
    }
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
