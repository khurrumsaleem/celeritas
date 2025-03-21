//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file orange/surf/FaceNamer.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <utility>

#include "orange/OrangeTypes.hh"

#include "SurfaceFwd.hh"
#include "VariantSurface.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Return a "name" for a face.
 *
 * This is generally a very short string to be used as an extension for a
 * volume comprised of several intersecting surface half-spaces. Because
 * successive surfaces should have separate names, this has a small internal
 * state.
 */
class FaceNamer
{
  public:
    // Construct with no prefix
    FaceNamer() = default;

    // Construct with prefix
    explicit inline FaceNamer(std::string&& prefix);

    // Apply to a surface with known type
    template<class S>
    inline std::string operator()(Sense s, S const& surf);

    // Apply to a surface with unknown type
    std::string operator()(Sense s, VariantSurface const& surf);

    // Apply with an explicit name
    inline std::string operator()(std::string const& s) const;

  private:
    struct State
    {
        int num_plane{0};
        int num_gq{0};
    };

    // String prefix
    std::string prefix_;

    // Persistent state
    State state_;

    // Nested implementation class
    struct Impl
    {
        State* state_;
        Sense sense_;

        //// OPERATORS ////

        template<Axis T>
        std::string operator()(PlaneAligned<T> const&) const;

        template<Axis T>
        std::string operator()(CylCentered<T> const&) const;

        std::string operator()(SphereCentered const&) const;

        template<Axis T>
        std::string operator()(CylAligned<T> const&) const;

        std::string operator()(Plane const&) const;

        std::string operator()(Sphere const&) const;

        template<Axis T>
        std::string operator()(ConeAligned<T> const&) const;

        std::string operator()(SimpleQuadric const&) const;

        std::string operator()(GeneralQuadric const&) const;

        std::string operator()(Involute const&) const;
    };
};

//---------------------------------------------------------------------------//
/*!
 * Construct with prefix.
 */
FaceNamer::FaceNamer(std::string&& prefix) : prefix_{std::move(prefix)}
{
    if (!prefix_.empty() && prefix_.back() != '.')
    {
        prefix_.push_back('.');
    }
}

//---------------------------------------------------------------------------//
/*!
 * Apply to a surface with known type.
 */
template<class S>
std::string FaceNamer::operator()(Sense s, S const& surf)
{
    std::string result = prefix_;
    result += Impl{&state_, s}(surf);
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Apply with an explicit face name.
 *
 * This can be useful for instances such as "generic trapezoid" where the
 * surface type can change based on whether the face is planar or twisted.
 */
std::string FaceNamer::operator()(std::string const& s) const
{
    std::string result = prefix_;
    result += s;
    return result;
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
