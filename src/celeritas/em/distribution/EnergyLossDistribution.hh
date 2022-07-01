//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/em/distribution/EnergyLossDistribution.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Assert.hh"

#include "EnergyLossHelper.hh"
#include "EnergyLossTraits.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
//! Return an energy loss distribution function based on the helper's model.
template<EnergyLossFluctuationModel M>
CELER_FUNCTION typename EnergyLossTraits<M>::type
make_distribution(const EnergyLossHelper& helper)
{
    CELER_EXPECT(helper.model() == M);

    using Distribution = typename EnergyLossTraits<M>::type;
    return Distribution{helper};
}

//---------------------------------------------------------------------------//
} // namespace celeritas