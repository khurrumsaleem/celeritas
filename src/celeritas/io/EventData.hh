//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/io/EventData.hh
//---------------------------------------------------------------------------//
#pragma once

#include <array>
#include <map>
#include <vector>

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Example of a calorimeter hit.
 */
struct EventHitData
{
    int volume{0};  //!< Logical volume ID
    int copy_num{0};  //!< Physical volume copy number
    double energy_dep{0};  //!< Energy deposition [MeV]
    double time{0};  //!< Pre-step global time [time]
};

//---------------------------------------------------------------------------//
/*!
 * Event data to be used within a Geant4/Celeritas offloading application.
 *
 * The hits are designed to be assigned to each sensitive volume, so that a
 * vector of hits of a given volume can be retrieved by doing
 *
 * \code
 * auto const& sd_hits = event_data.hits[sensdet_id];
 * for (auto const& hit : sd_hits)
 * {
 *     // Access hit information from this given detector in this event.
 * }
 * \endcode
 *
 * Therefore, sensitive detector IDs must be contiguously assigned and mapped
 * to their sensitive detector name at startup.
 */
struct EventData
{
    int event_id{0};
    std::vector<std::vector<EventHitData>> hits;
};

//---------------------------------------------------------------------------//
}  // namespace celeritas
