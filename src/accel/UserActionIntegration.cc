//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file accel/UserActionIntegration.cc
//---------------------------------------------------------------------------//
#include "UserActionIntegration.hh"

#include <G4Event.hh>
#include <G4Threading.hh>
#include <G4Track.hh>

#include "corecel/sys/Stopwatch.hh"

#include "ExceptionConverter.hh"
#include "TimeOutput.hh"

#include "detail/IntegrationSingleton.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Access the singleton.
 */
UserActionIntegration& UserActionIntegration::Instance()
{
    static UserActionIntegration uai;
    return uai;
}

//---------------------------------------------------------------------------//
/*!
 * Start the run.
 */
void UserActionIntegration::BeginOfRunAction(G4Run const*)
{
    Stopwatch get_setup_time;

    auto& singleton = detail::IntegrationSingleton::instance();

    if (G4Threading::IsMasterThread())
    {
        singleton.initialize_shared_params();
    }

    singleton.initialize_local_transporter();

    if (G4Threading::IsMasterThread())
    {
        singleton.shared_params().timer()->RecordSetupTime(get_setup_time());
        singleton.start_timer();
    }
}

//---------------------------------------------------------------------------//
/*!
 * Send Celeritas the event ID.
 */
void UserActionIntegration::BeginOfEventAction(G4Event const* event)
{
    get_event_time_ = {};

    auto& local = detail::IntegrationSingleton::local_transporter();
    if (!local)
        return;

    // Set event ID in local transporter and reseed RNG for reproducibility
    CELER_TRY_HANDLE(local.InitializeEvent(event->GetEventID()),
                     ExceptionConverter{"celer.event.begin"});
}

//---------------------------------------------------------------------------//
/*!
 * Send tracks to Celeritas if applicable and "StopAndKill" if so.
 */
void UserActionIntegration::PreUserTrackingAction(G4Track* track)
{
    CELER_EXPECT(track);

    auto& singleton = detail::IntegrationSingleton::instance();
    auto const mode = singleton.shared_params().mode();
    if (mode == SharedParams::Mode::disabled)
        return;

    auto const& particles = singleton.shared_params().OffloadParticles();
    if (std::find(particles.begin(), particles.end(), track->GetDefinition())
        != particles.end())
    {
        if (mode == SharedParams::Mode::enabled)
        {
            // Celeritas is transporting this track
            CELER_TRY_HANDLE(
                detail::IntegrationSingleton::local_transporter().Push(*track),
                ExceptionConverter("celer.track.push",
                                   &singleton.shared_params()));
        }

        // Either "pushed" or we're in kill_offload mode
        track->SetTrackStatus(fStopAndKill);
    }
}

//---------------------------------------------------------------------------//
/*!
 * Flush offloaded tracks from Celeritas.
 */
void UserActionIntegration::EndOfEventAction(G4Event const*)
{
    auto& local = detail::IntegrationSingleton::local_transporter();
    if (!local)
        return;

    auto& singleton = detail::IntegrationSingleton::instance();
    CELER_TRY_HANDLE(
        local.Flush(),
        ExceptionConverter("celer.event.flush", &singleton.shared_params()));

    // Record the time for this event
    singleton.shared_params().timer()->RecordEventTime(get_event_time_());
}

//---------------------------------------------------------------------------//
/*!
 * Only allow the singleton to construct.
 */
UserActionIntegration::UserActionIntegration() = default;

//---------------------------------------------------------------------------//
}  // namespace celeritas
