//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/CerenkovPreGenerator.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Assert.hh"
#include "corecel/Macros.hh"
#include "celeritas/phys/ParticleTrackView.hh"
#include "celeritas/random/distribution/PoissonDistribution.hh"
#include "celeritas/track/SimTrackView.hh"

#include "CerenkovData.hh"
#include "CerenkovDndxCalculator.hh"
#include "OpticalDistributionData.hh"
#include "OpticalGenData.hh"
#include "OpticalPropertyData.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Sample the number of Cerenkov photons to be generated.
 *
 * This populates the \c OpticalDistributionData used by the \c
 * CerenkovGenerator to generate optical photons using post-step and cached
 * pre-step data.
 */
class CerenkovPreGenerator
{
  public:
    // Construct with optical properties, Cerenkov, and step data
    inline CELER_FUNCTION
    CerenkovPreGenerator(ParticleTrackView const& particle,
                         SimTrackView const& sim,
                         Real3 const& pos,
                         NativeCRef<OpticalPropertyData> const& properties,
                         NativeCRef<CerenkovData> const& shared,
                         OpticalPreStepData const& step_data);

    // Return a populated optical distribution data for the Cerenkov Generator
    template<class Generator>
    inline CELER_FUNCTION OpticalDistributionData operator()(Generator& rng);

  private:
    units::ElementaryCharge charge_;
    real_type step_length_;
    OpticalPreStepData const& pre_step_;
    OpticalStepData post_step_;
    real_type num_photons_per_len_;
};

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Construct with optical properties, Cerenkov, and step information.
 */
CELER_FUNCTION CerenkovPreGenerator::CerenkovPreGenerator(
    ParticleTrackView const& particle,
    SimTrackView const& sim,
    Real3 const& pos,
    NativeCRef<OpticalPropertyData> const& properties,
    NativeCRef<CerenkovData> const& shared,
    OpticalPreStepData const& step_data)
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

    CerenkovDndxCalculator calculate_dndx(
        properties, shared, pre_step_.opt_mat, charge_);
    num_photons_per_len_ = calculate_dndx(beta);
}

//---------------------------------------------------------------------------//
/*!
 * Return an \c OpticalDistributionData object. If no photons are sampled, an
 * empty object is returned and can be verified via its own operator bool.
 *
 * The number of photons is sampled from a Poisson distribution with a mean
 * \f[
   \langle n \rangle = \ell_\text{step} \frac{dN}{dx}
 * \f]
 * where \f$ \ell_\text{step} \f$ is the step length.
 */
template<class Generator>
CELER_FUNCTION OpticalDistributionData
CerenkovPreGenerator::operator()(Generator& rng)
{
    if (num_photons_per_len_ == 0)
    {
        return {};
    }

    OpticalDistributionData data;
    data.num_photons = PoissonDistribution<real_type>(num_photons_per_len_
                                                      * step_length_)(rng);
    if (data.num_photons > 0)
    {
        data.time = pre_step_.time;
        data.step_length = step_length_;
        data.charge = charge_;
        data.material = pre_step_.opt_mat;
        data.points[StepPoint::pre].speed = pre_step_.speed;
        data.points[StepPoint::pre].pos = pre_step_.pos;
        data.points[StepPoint::post] = post_step_;
    }
    return data;
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
