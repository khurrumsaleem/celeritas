//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/phys/ParticleParamsOutput.cc
//---------------------------------------------------------------------------//
#include "ParticleParamsOutput.hh"

#include <utility>
#include <nlohmann/json.hpp>

#include "corecel/Config.hh"

#include "corecel/io/JsonPimpl.hh"
#include "corecel/math/Quantity.hh"

#include "ParticleParams.hh"  // IWYU pragma: keep
#include "Process.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Construct from shared particle data.
 */
ParticleParamsOutput::ParticleParamsOutput(SPConstParticleParams particles)
    : particles_(std::move(particles))
{
    CELER_EXPECT(particles_);
}

//---------------------------------------------------------------------------//
/*!
 * Write output to the given JSON object.
 */
void ParticleParamsOutput::output(JsonPimpl* j) const
{
    using json = nlohmann::json;
    auto units = json::object();
    auto label = json::array();
    auto pdg = json::array();
    auto mass = json::array();
    auto charge = json::array();
    auto decay_constant = json::array();
    auto is_antiparticle = json::array();

    for (auto id : range(ParticleId{particles_->size()}))
    {
        label.push_back(particles_->id_to_label(id));
        pdg.push_back(particles_->id_to_pdg(id).unchecked_get());

        ParticleView const par_view = particles_->get(id);
        mass.push_back(par_view.mass().value());
        charge.push_back(par_view.charge().value());
        decay_constant.push_back(par_view.decay_constant());
        is_antiparticle.push_back(par_view.is_antiparticle());
    }

    j->obj = {
        {"_units",
         {
             {"mass", accessor_unit_label<decltype(&ParticleView::mass)>()},
             {"charge", accessor_unit_label<decltype(&ParticleView::charge)>()},
         }},
        {"label", std::move(label)},
        {"pdg", std::move(pdg)},
        {"mass", std::move(mass)},
        {"charge", std::move(charge)},
        {"decay_constant", std::move(decay_constant)},
        {"is_antiparticle", std::move(is_antiparticle)},
    };
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
