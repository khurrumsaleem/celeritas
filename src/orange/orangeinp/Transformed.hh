//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file orange/orangeinp/Transformed.hh
//---------------------------------------------------------------------------//
#pragma once

#include "orange/transform/VariantTransform.hh"

#include "ObjectInterface.hh"

namespace celeritas
{
namespace orangeinp
{
//---------------------------------------------------------------------------//
/*!
 * Build a translated or transformed object.
 */
class Transformed final : public ObjectInterface
{
  public:
    // Construct a transformed object if nontrivial, or return the original
    static SPConstObject
    or_object(SPConstObject obj, VariantTransform const& transform);

    // Construct with daughter object and transform
    Transformed(SPConstObject obj, VariantTransform const& transform);

    //! Access the daughter object
    SPConstObject const& daughter() const { return obj_; }

    //! Access the transform
    VariantTransform const& transform() const { return transform_; }

    //! Get the user-provided label
    std::string_view label() const final { return obj_->label(); }

    // Construct a volume from this object
    NodeId build(VolumeBuilder&) const final;

    // Write the object to JSON
    void output(JsonPimpl*) const final;

  private:
    SPConstObject obj_;
    VariantTransform transform_;
};

//---------------------------------------------------------------------------//
}  // namespace orangeinp
}  // namespace celeritas
