//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/CerenkovParams.cc
//---------------------------------------------------------------------------//
#include "CerenkovParams.hh"

#include <utility>
#include <vector>

#include "corecel/cont/Range.hh"
#include "corecel/data/CollectionBuilder.hh"
#include "corecel/data/DedupeCollectionBuilder.hh"
#include "corecel/math/Algorithms.hh"
#include "celeritas/Quantities.hh"
#include "celeritas/Types.hh"
#include "celeritas/grid/GenericGridInserter.hh"

#include "MaterialParams.hh"

namespace celeritas
{
namespace optical
{
//---------------------------------------------------------------------------//
/*!
 * Construct with optical property data.
 */
CerenkovParams::CerenkovParams(SPConstMaterial material)
{
    CELER_EXPECT(material);
    auto const& host_ref = material->host_ref();

    HostVal<CerenkovData> data;
    GenericGridInserter insert_angle_integral(&data.reals,
                                              &data.angle_integral);

    for (auto mat_id :
         range(OpticalMaterialId(host_ref.refractive_index.size())))
    {
        auto const& ri_grid = host_ref.refractive_index[mat_id];
        CELER_ASSERT(ri_grid);

        // Calculate the Cerenkov angle integral
        auto const&& refractive_index = host_ref.reals[ri_grid.value];
        auto const&& energy = host_ref.reals[ri_grid.grid];
        std::vector<real_type> integral(energy.size());
        for (size_type i = 1; i < energy.size(); ++i)
        {
            integral[i] = integral[i - 1]
                          + real_type(0.5) * (energy[i] - energy[i - 1])
                                * (1 / ipow<2>(refractive_index[i - 1])
                                   + 1 / ipow<2>(refractive_index[i]));
        }

        insert_angle_integral(make_span(energy), make_span(integral));
    }
    CELER_ASSERT(data.angle_integral.size()
                 == host_ref.refractive_index.size());

    data_ = CollectionMirror<CerenkovData>{std::move(data)};
    CELER_ENSURE(data_ || host_ref.refractive_index.empty());
}

//---------------------------------------------------------------------------//
}  // namespace optical
}  // namespace celeritas
