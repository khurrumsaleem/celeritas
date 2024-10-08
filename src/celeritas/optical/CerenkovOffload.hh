//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/CerenkovOffload.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Assert.hh"
#include "corecel/Macros.hh"
#include "celeritas/phys/ParticleTrackView.hh"
#include "celeritas/random/distribution/PoissonDistribution.hh"
#include "celeritas/track/SimTrackView.hh"

#include "CerenkovData.hh"
#include "CerenkovDndxCalculator.hh"
#include "GeneratorDistributionData.hh"
#include "MaterialView.hh"
#include "OffloadData.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Sample the number of Cerenkov photons to be generated.
 *
 * This populates the \c GeneratorDistributionData used by the \c
 * CerenkovGenerator to generate optical photons using post-step and cached
 * pre-step data.
 *
 * The number of photons is sampled from a Poisson distribution with a mean
 * \f[
   \langle n \rangle = \ell_\text{step} \difd{N}{x}
 * \f]
 * where \f$ \ell_\text{step} \f$ is the step length.
 */
class CerenkovOffload
{
  public:
    // Construct with optical material, Cerenkov, and step data
    inline CELER_FUNCTION
    CerenkovOffload(ParticleTrackView const& particle,
                    SimTrackView const& sim,
                    optical::MaterialView const& mat,
                    Real3 const& pos,
                    NativeCRef<optical::CerenkovData> const& shared,
                    OffloadPreStepData const& step_data);

    // Return a populated optical distribution data for the Cerenkov Generator
    template<class Generator>
    inline CELER_FUNCTION optical::GeneratorDistributionData
    operator()(Generator& rng);

  private:
    units::ElementaryCharge charge_;
    real_type step_length_;
    OffloadPreStepData const& pre_step_;
    optical::GeneratorStepData post_step_;
    real_type num_photons_per_len_;
};

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Construct with optical material, Cerenkov, and step information.
 */
CELER_FUNCTION
CerenkovOffload::CerenkovOffload(ParticleTrackView const& particle,
                                 SimTrackView const& sim,
                                 optical::MaterialView const& mat,
                                 Real3 const& pos,
                                 NativeCRef<optical::CerenkovData> const& shared,
                                 OffloadPreStepData const& step_data)
    : charge_(particle.charge())
    , step_length_(sim.step_length())
    , pre_step_(step_data)
    , post_step_({particle.speed(), pos})
{
    CELER_EXPECT(charge_ != zero_quantity());
    CELER_EXPECT(step_length_ > 0);
    CELER_EXPECT(pre_step_);

    units::LightSpeed beta(
        real_type{0.5} * (pre_step_.speed.value() + post_step_.speed.value()));

    optical::CerenkovDndxCalculator calculate_dndx(mat, shared, charge_);
    num_photons_per_len_ = calculate_dndx(beta);
}

//---------------------------------------------------------------------------//
/*!
 * Return an \c GeneratorDistributionData object.
 *
 * If no photons are sampled, an empty object is returned and can be verified
 * via its own operator bool.
 */
template<class Generator>
CELER_FUNCTION optical::GeneratorDistributionData
CerenkovOffload::operator()(Generator& rng)
{
    if (num_photons_per_len_ == 0)
    {
        return {};
    }

    optical::GeneratorDistributionData data;
    data.num_photons = PoissonDistribution<real_type>(num_photons_per_len_
                                                      * step_length_)(rng);
    if (data.num_photons > 0)
    {
        data.time = pre_step_.time;
        data.step_length = step_length_;
        data.charge = charge_;
        data.material = pre_step_.material;
        data.points[StepPoint::pre].speed = pre_step_.speed;
        data.points[StepPoint::pre].pos = pre_step_.pos;
        data.points[StepPoint::post] = post_step_;
    }
    return data;
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
