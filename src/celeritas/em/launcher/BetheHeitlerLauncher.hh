//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/em/launcher/BetheHeitlerLauncher.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Assert.hh"
#include "corecel/Macros.hh"
#include "celeritas/em/data/BetheHeitlerData.hh"
#include "celeritas/em/interactor/BetheHeitlerInteractor.hh"
#include "celeritas/global/CoreTrackView.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Apply BetheHeitler to the current track.
 */
inline CELER_FUNCTION Interaction bethe_heitler_interact_track(
    BetheHeitlerData const& model, CoreTrackView const& track)
{
    auto material_track = track.make_material_view();
    auto material       = material_track.make_material_view();
    auto particle       = track.make_particle_view();

    auto elcomp_id = track.make_physics_step_view().element();
    CELER_ASSERT(elcomp_id);
    auto element = material.make_element_view(elcomp_id);
    auto allocate_secondaries
        = track.make_physics_step_view().make_secondary_allocator();
    const auto& dir = track.make_geo_view().dir();

    BetheHeitlerInteractor interact(
        model, particle, dir, allocate_secondaries, material, element);

    auto rng = track.make_rng_engine();
    return interact(rng);
}

//---------------------------------------------------------------------------//
} // namespace celeritas