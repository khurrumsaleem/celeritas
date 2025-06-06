//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/gen/CherenkovOffload.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Assert.hh"
#include "corecel/Macros.hh"
#include "corecel/random/distribution/PoissonDistribution.hh"
#include "celeritas/phys/ParticleTrackView.hh"
#include "celeritas/track/SimTrackView.hh"

#include "CherenkovData.hh"
#include "CherenkovDndxCalculator.hh"
#include "GeneratorData.hh"
#include "OffloadData.hh"
#include "../MaterialView.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Sample the number of Cherenkov photons to be generated.
 *
 * This populates the \c GeneratorDistributionData used by the \c
 * CherenkovGenerator to generate optical photons using post-step and cached
 * pre-step data.
 *
 * The number of photons is sampled from a Poisson distribution with a mean
 * \f[
   \langle n \rangle = \ell_\text{step} \difd{N}{x}
 * \f]
 * where \f$ \ell_\text{step} \f$ is the step length.
 */
class CherenkovOffload
{
  public:
    // Construct with optical material, Cherenkov, and step data
    inline CELER_FUNCTION
    CherenkovOffload(ParticleTrackView const& particle,
                     SimTrackView const& sim,
                     optical::MaterialView const& mat,
                     Real3 const& pos,
                     NativeCRef<CherenkovData> const& shared,
                     OffloadPreStepData const& step_data);

    // Gather the input data needed to sample Cherenkov photons
    template<class Generator>
    inline CELER_FUNCTION GeneratorDistributionData operator()(Generator& rng);

  private:
    units::ElementaryCharge charge_;
    real_type step_length_;
    OffloadPreStepData const& pre_step_;
    GeneratorStepData post_step_;
    real_type num_photons_per_len_;
};

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Construct with optical material, Cherenkov, and step information.
 */
CELER_FUNCTION
CherenkovOffload::CherenkovOffload(ParticleTrackView const& particle,
                                   SimTrackView const& sim,
                                   optical::MaterialView const& mat,
                                   Real3 const& pos,
                                   NativeCRef<CherenkovData> const& shared,
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

    CherenkovDndxCalculator calculate_dndx(mat, shared, charge_);
    num_photons_per_len_ = calculate_dndx(beta);
}

//---------------------------------------------------------------------------//
/*!
 * Collect the distribution data needed to sample Cherenkov photons.
 *
 * If no photons are sampled an empty object is returned.
 */
template<class Generator>
CELER_FUNCTION GeneratorDistributionData
CherenkovOffload::operator()(Generator& rng)
{
    if (num_photons_per_len_ == 0)
    {
        return {};
    }

    GeneratorDistributionData data;
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
