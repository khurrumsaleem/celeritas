//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/phys/PrimaryGeneratorOptionsIO.json.cc
//---------------------------------------------------------------------------//
#include "PrimaryGeneratorOptionsIO.json.hh"

#include <algorithm>
#include <string>
#include <vector>

#include "corecel/Assert.hh"
#include "corecel/cont/ArrayIO.json.hh"
#include "corecel/cont/Range.hh"
#include "corecel/io/JsonUtils.json.hh"
#include "corecel/io/Logger.hh"
#include "corecel/io/StringEnumMapper.hh"

#include "PDGNumber.hh"
#include "PrimaryGeneratorOptions.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
static char const format_str[] = "primary-generator";

//---------------------------------------------------------------------------//
// JSON serializers
//---------------------------------------------------------------------------//
void from_json(nlohmann::json const& j, DistributionSelection& value)
{
    static auto from_string
        = StringEnumMapper<DistributionSelection>::from_cstring_func(
            to_cstring, "distribution type");
    value = from_string(j.get<std::string>());
}

void to_json(nlohmann::json& j, DistributionSelection const& value)
{
    j = std::string{to_cstring(value)};
}

void from_json(nlohmann::json const& j, DistributionOptions& opts)
{
    j.at("distribution").get_to(opts.distribution);
    if (j.contains("params"))
    {
        j.at("params").get_to(opts.params);
    }
}

void to_json(nlohmann::json& j, DistributionOptions const& opts)
{
    if (!opts)
    {
        j = nlohmann::json::object();
        return;
    }
    j = nlohmann::json{{"distribution", opts.distribution},
                       {"params", opts.params}};
}

//---------------------------------------------------------------------------//
/*!
 * Read options from JSON.
 */
void from_json(nlohmann::json const& j, PrimaryGeneratorOptions& opts)
{
    check_format(j, format_str);
    check_units(j, format_str);

    if (auto iter = j.find("seed"); iter != j.end())
    {
        iter->get_to(opts.seed);
    }
    else
    {
        CELER_LOG(warning) << "Primary generator options are missing 'seed': "
                              "defaulting to "
                           << opts.seed;
    }
    std::vector<int> pdg;
    auto&& pdg_input = j.at("pdg");
    if (pdg_input.is_array())
    {
        pdg_input.get_to(pdg);
    }
    else
    {
        // Backward compatibility: single PDG
        pdg = {pdg_input.get<int>()};
    }
    opts.pdg.reserve(pdg.size());
    for (int i : pdg)
    {
        PDGNumber p{i};
        opts.pdg.push_back(p);
        CELER_VALIDATE(p, << "invalid PDG number " << i);
    }

    CELER_JSON_LOAD_REQUIRED(j, opts, num_events);
    CELER_JSON_LOAD_REQUIRED(j, opts, primaries_per_event);

    auto&& energy_input = j.at("energy");
    if (energy_input.is_object())
    {
        energy_input.get_to(opts.energy);
    }
    else
    {
        // Backward compatibility: monoenergetic energy
        opts.energy.distribution = DistributionSelection::delta;
        opts.energy.params = {energy_input.get<real_type>()};
    }
    auto&& pos_input = j.at("position");
    if (pos_input.is_object())
    {
        pos_input.get_to(opts.position);
    }
    else
    {
        // Backward compatibility: point source
        opts.position.distribution = DistributionSelection::delta;
        pos_input.get_to(opts.position.params);
    }
    auto&& dir_input = j.at("direction");
    if (dir_input.is_object())
    {
        dir_input.get_to(opts.direction);
    }
    else
    {
        // Backward compatibility: point source
        opts.direction.distribution = DistributionSelection::delta;
        dir_input.get_to(opts.direction.params);
    }
}

//---------------------------------------------------------------------------//
/*!
 * Write options to JSON.
 */
void to_json(nlohmann::json& j, PrimaryGeneratorOptions const& opts)
{
    std::vector<int> pdg(opts.pdg.size());
    std::transform(
        opts.pdg.begin(), opts.pdg.end(), pdg.begin(), [](PDGNumber p) {
            return p.unchecked_get();
        });

    j = nlohmann::json{
        {"pdg", pdg},
        CELER_JSON_PAIR(opts, seed),
        CELER_JSON_PAIR(opts, num_events),
        CELER_JSON_PAIR(opts, primaries_per_event),
        CELER_JSON_PAIR(opts, energy),
        CELER_JSON_PAIR(opts, position),
        CELER_JSON_PAIR(opts, direction),
    };

    save_format(j, format_str);
    save_units(j);
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
