//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file orange/detail/BIHUtils.hh
//---------------------------------------------------------------------------//
#pragma once

#include <vector>

#include "../BoundingBoxUtils.hh"
#include "../OrangeTypes.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Calculate bounding box enclosing bounding boxes for specified indices.
 */
inline FastBBox calc_union(std::vector<FastBBox> const& bboxes,
                           std::vector<LocalVolumeId> const& indices)
{
    FastBBox result;
    for (auto const& id : indices)
    {
        CELER_ASSERT(id < bboxes.size());
        result = calc_union(result, bboxes[id.unchecked_get()]);
    }

    return result;
}
//---------------------------------------------------------------------------//
}  // namespace celeritas
