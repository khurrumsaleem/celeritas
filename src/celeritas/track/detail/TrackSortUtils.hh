//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/track/detail/TrackSortUtils.hh
//---------------------------------------------------------------------------//
#pragma once

#include <type_traits>

#include "corecel/Assert.hh"
#include "corecel/Macros.hh"
#include "corecel/Types.hh"
#include "corecel/cont/Span.hh"
#include "corecel/data/ObserverPtr.hh"
#include "corecel/sys/ThreadId.hh"
#include "celeritas/global/CoreTrackData.hh"

namespace celeritas
{
namespace detail
{
//---------------------------------------------------------------------------//
// HELPER FUNCTIONS
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Sort or partition tracks
void sort_tracks(HostRef<CoreStateData> const&, TrackOrder);
void sort_tracks(DeviceRef<CoreStateData> const&, TrackOrder);

//---------------------------------------------------------------------------//
// Count tracks associated to each action
void count_tracks_per_action(
    HostRef<CoreStateData> const&,
    Span<ThreadId>,
    Collection<ThreadId, Ownership::value, MemSpace::host, ActionId>&,
    TrackOrder);

void count_tracks_per_action(
    DeviceRef<CoreStateData> const&,
    Span<ThreadId>,
    Collection<ThreadId, Ownership::value, MemSpace::mapped, ActionId>&,
    TrackOrder);

//---------------------------------------------------------------------------//
// Fill missing action offsets.
void backfill_action_count(Span<ThreadId>, size_type);

//---------------------------------------------------------------------------//
// HELPER CLASSES AND FUNCTIONS
//---------------------------------------------------------------------------//
//! Uses as a predicate to sort inactive tracks from active
struct IsNotInactive
{
    ObserverPtr<TrackStatus const> status_;

    CELER_FUNCTION bool operator()(size_type track_slot) const
    {
        return status_.get()[track_slot] != TrackStatus::inactive;
    }
};

//! Map from a thread ID to an action ID by pointer indirection
struct ActionAccessor
{
    ObserverPtr<ActionId const> action_;
    ObserverPtr<TrackSlotId::size_type const> track_slots_;

    CELER_FUNCTION ActionId operator()(ThreadId tid) const
    {
        return action_.get()[track_slots_.get()[tid.get()]];
    }
};

//---------------------------------------------------------------------------//
//! Return a raw pointer to action IDs based on the given sort order
template<Ownership W, MemSpace M>
CELER_FUNCTION ObserverPtr<ActionId const>
get_action_ptr(CoreStateData<W, M> const& states, TrackOrder order)
{
    if (order == TrackOrder::reindex_along_step_action)
    {
        return states.sim.along_step_action.data();
    }
    else if (order == TrackOrder::reindex_step_limit_action)
    {
        return states.sim.post_step_action.data();
    }
    CELER_ASSERT_UNREACHABLE();
}

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
#if !CELER_USE_DEVICE
inline void sort_tracks(DeviceRef<CoreStateData> const&, TrackOrder)
{
    CELER_NOT_CONFIGURED("CUDA or HIP");
}

inline void count_tracks_per_action(
    DeviceRef<CoreStateData> const&,
    Span<ThreadId>,
    Collection<ThreadId, Ownership::value, MemSpace::mapped, ActionId>&,
    TrackOrder)
{
    CELER_NOT_CONFIGURED("CUDA or HIP");
}
#endif
//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
