//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/inp/Events.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <variant>

#include "corecel/Types.hh"
#include "geocel/Types.hh"
#include "celeritas/Quantities.hh"
#include "celeritas/phys/PDGNumber.hh"

namespace celeritas
{
namespace inp
{
//---------------------------------------------------------------------------//
//! Generate at a single point
struct PointShape
{
    Real3 pos{0, 0, 0};
};

//! Sample uniformly in a box
struct UniformBoxShape
{
    Real3 lower{0, 0, 0};
    Real3 upper{0, 0, 0};
};

//! Choose a spatial distribution for the primary generator
using ShapeDistribution = std::variant<PointShape, UniformBoxShape>;

//---------------------------------------------------------------------------//
//! Generate angles isotropically
struct IsotropicAngle
{
};

//! Generate angles in a single direction
struct MonodirectionalAngle
{
    Real3 dir{0, 0, 1};
};

//! Choose an angular distribution for the primary generator
using AngleDistribution = std::variant<IsotropicAngle, MonodirectionalAngle>;

//---------------------------------------------------------------------------//
//! Generate primaries at a single energy value
struct Monoenergetic
{
    units::MevEnergy energy;
};

//! Choose an angular distribution for the primary generator
using EnergyDistribution = Monoenergetic;

//---------------------------------------------------------------------------//
/*!
 * Generate from a hardcoded distribution of primary particles.
 *
 * \todo Allow programmatic setting from particle ID as well
 * \todo Units?
 * \code using Particle = std::variant<PDGNumber, ParticleId>; \endcode
 */
struct PrimaryGenerator
{
    //! Random number seed
    unsigned int seed{};
    //! Sample evenly from this vector of particle types
    std::vector<PDGNumber> pdg;
    //! Number of events to generate
    size_type num_events{};
    //! Number of primaries per event
    size_type primaries_per_event{};

    //! Distribution for sampling source position
    ShapeDistribution shape;
    //! Distribution for sampling source direction
    AngleDistribution angle;
    //! Distribution for sampling source energy
    EnergyDistribution energy;
};

//---------------------------------------------------------------------------//
//! Sample random events from an input file
struct SampleFileEvents
{
    //! Total number of events to sample
    size_type num_events{};
    //! File events per sampled event
    size_type num_merged{};

    //! ROOT file input
    std::string event_file;

    //! Random number generator seed
    unsigned int seed{};
};

//---------------------------------------------------------------------------//
//! Read all events from the given file
struct ReadFileEvents
{
    std::string event_file;
};

//---------------------------------------------------------------------------//
//! Mechanism for generating events for tracking
using Events = std::variant<PrimaryGenerator, SampleFileEvents, ReadFileEvents>;

//---------------------------------------------------------------------------//
}  // namespace inp
}  // namespace celeritas
