//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file orange/transform/VariantTransform.hh
//---------------------------------------------------------------------------//
#pragma once

#include <variant>

#include "corecel/cont/VariantUtils.hh"

#include "NoTransformation.hh"
#include "SignedPermutation.hh"
#include "TransformTypeTraits.hh"
#include "Transformation.hh"
#include "Translation.hh"

namespace celeritas
{
template<class T>
class BoundingBox;

//---------------------------------------------------------------------------//
//! std::variant for all transforms.
using VariantTransform = EnumVariant<TransformType, TransformTypeTraits>;

//---------------------------------------------------------------------------//
// Apply the left "daughter-to-parent" transform to the right
[[nodiscard]] VariantTransform
apply_transform(VariantTransform const& left, VariantTransform const& right);

//---------------------------------------------------------------------------//
// Calculate the inverse of a transform
[[nodiscard]] VariantTransform calc_inverse(VariantTransform const& transform);

//---------------------------------------------------------------------------//
// Dispatch "daughter-to-parent" transform to bounding box utilities
[[nodiscard]] BoundingBox<real_type>
apply_transform(VariantTransform const& transform,
                BoundingBox<real_type> const& bbox);

//---------------------------------------------------------------------------//
}  // namespace celeritas
