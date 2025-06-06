//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/phys/RootEventSampler.cc
//---------------------------------------------------------------------------//
#include "RootEventSampler.hh"

#include <algorithm>

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Construct \c RootEventReader , and initialize sampling conditions.
 */
RootEventSampler::RootEventSampler(std::string const& filename,
                                   SPConstParticles particles,
                                   size_type num_sampled_events,
                                   size_type num_merged_events,
                                   unsigned int seed)
    : num_sampled_events_(num_sampled_events)
    , num_merged_events_(num_merged_events)
{
    CELER_EXPECT(!filename.empty());
    CELER_EXPECT(particles);

    reader_ = std::make_unique<RootEventReader>(filename, std::move(particles));
    CELER_EXPECT(num_merged_events_ > 0
                 && num_merged_events_ <= reader_->num_events());

    rng_.seed(seed);
    select_event_
        = std::uniform_int_distribution<size_type>(0, reader_->num_events());
}

//---------------------------------------------------------------------------//
/*!
 * Return a vector of sampled primaries.
 */
auto RootEventSampler::operator()() -> result_type
{
    if (event_count_.get() == num_sampled_events_)
    {
        return {};
    }

    result_type result;
    for ([[maybe_unused]] auto i : range(num_merged_events_))
    {
        // Select a random event and overwrite its event id
        auto event = reader_->operator()(EventId{select_event_(rng_)});
        std::for_each(
            event.begin(), event.end(), [ec = event_count_](auto& primary) {
                primary.event_id = ec;
            });
        result.insert(result.end(), event.begin(), event.end());
    }
    event_count_++;

    return result;
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
