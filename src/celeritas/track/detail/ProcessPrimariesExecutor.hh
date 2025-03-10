//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/track/detail/ProcessPrimariesExecutor.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Assert.hh"
#include "corecel/Macros.hh"
#include "corecel/cont/Span.hh"
#include "celeritas/Quantities.hh"
#include "celeritas/Types.hh"
#include "celeritas/global/CoreTrackData.hh"
#include "celeritas/phys/ParticleData.hh"
#include "celeritas/phys/Primary.hh"

#include "Utils.hh"
#include "../SimData.hh"
#include "../TrackInitData.hh"

namespace celeritas
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Create track initializers from primary particles.
 */
struct ProcessPrimariesExecutor
{
    //// TYPES ////

    using ParamsPtr = CRefPtr<CoreParamsData, MemSpace::native>;
    using StatePtr = RefPtr<CoreStateData, MemSpace::native>;

    //// DATA ////

    ParamsPtr params;
    StatePtr state;
    CoreStateCounters counters;

    Span<Primary const> primaries;

    //// FUNCTIONS ////

    // Create track initializers from primaries
    inline CELER_FUNCTION void operator()(ThreadId tid) const;
};

//---------------------------------------------------------------------------//
/*!
 * Create track initializers from primaries.
 */
CELER_FUNCTION void ProcessPrimariesExecutor::operator()(ThreadId tid) const
{
    CELER_EXPECT(tid < primaries.size());
    CELER_EXPECT(primaries.size() <= counters.num_initializers + tid.get());

    ItemId<TrackInitializer> idx{
        index_after(counters.num_initializers - primaries.size(), tid)};
    TrackInitializer& ti = state->init.initializers[idx];
    Primary const& primary = primaries[tid.unchecked_get()];

    // Construct a track initializer from a primary particle
    ti.sim.track_id
        = make_track_id(params->init, state->init, primary.event_id);
    ti.sim.parent_id = TrackId{};
    ti.sim.event_id = primary.event_id;
    ti.sim.time = primary.time;
    ti.geo.pos = primary.position;
    ti.geo.dir = primary.direction;
    ti.particle.particle_id = primary.particle_id;
    ti.particle.energy = primary.energy;
}

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
