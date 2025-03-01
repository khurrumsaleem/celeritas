//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file geocel/Types.hh
//! Shared (VecGeom + ORANGE) geometry type definitions.
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/OpaqueId.hh"
#include "corecel/Types.hh"
#include "corecel/cont/Array.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
// TYPE ALIASES
//---------------------------------------------------------------------------//

//! Three-dimensional cartesian coordinates
using Real3 = Array<real_type, 3>;

//! Two-dimensional cartesian coordinates
using Real2 = Array<real_type, 2>;

//! Two-dimensional extents
using Size2 = Array<size_type, 2>;

//! Alias for a small square dense matrix
template<class T, size_type N>
using SquareMatrix = Array<Array<T, N>, N>;

//! Alias for a small square dense matrix
using SquareMatrixReal3 = SquareMatrix<real_type, 3>;

//---------------------------------------------------------------------------//

//! Type-safe "level", i.e., depth of embedded unit/scene/volume
using LevelId = OpaqueId<struct Level_>;

//! Identifier for a material fill
using GeoMaterialId = OpaqueId<struct GeoMaterial_>;

//! Identifier for a surface (for surface-based geometries)
using SurfaceId = OpaqueId<struct Surface_>;

//! Identifier for a geometry volume that may be repeated
using VolumeId = OpaqueId<struct Volume_>;

//! Identifier for an instance of a geometry volume (aka physical/placed)
using VolumeInstanceId = OpaqueId<struct VolumeInstance_>;

//---------------------------------------------------------------------------//
// ENUMERATIONS
//---------------------------------------------------------------------------//
/*!
 * Enumeration for cartesian axes.
 */
enum class Axis
{
    x,  //!< X axis/I index coordinate
    y,  //!< Y axis/J index coordinate
    z,  //!< Z axis/K index coordinate
    size_  //!< Sentinel value for looping over axes
};

//---------------------------------------------------------------------------//
/*!
 * Which of two bounding points, faces, etc.
 *
 * Here, lo/hi correspond to left/right, back/front, bottom/top. It's used for
 * the two points in a bounding box.
 */
enum class Bound : unsigned char
{
    lo,
    hi,
    size_
};

//---------------------------------------------------------------------------//
// STRUCTS
//---------------------------------------------------------------------------//
/*!
 * Data required to initialize a geometry state.
 */
struct GeoTrackInitializer
{
    Real3 pos{0, 0, 0};
    Real3 dir{0, 0, 0};

    //! True if assigned
    explicit CELER_FUNCTION operator bool() const
    {
        return dir[0] != 0 || dir[1] != 0 || dir[2] != 0;
    }
};

//---------------------------------------------------------------------------//
/*!
 * Result of a propagation step.
 *
 * The boundary flag means that the geometry is step limiting, but the surface
 * crossing must be called externally.
 */
struct Propagation
{
    real_type distance{0};  //!< Distance traveled
    bool boundary{false};  //!< True if hit a boundary before given distance
    bool looping{false};  //!< True if track is looping in the field propagator
};

//---------------------------------------------------------------------------//
// HELPER FUNCTIONS
//---------------------------------------------------------------------------//
//! Convert Axis enum value to int
CELER_CONSTEXPR_FUNCTION int to_int(Axis a)
{
    return static_cast<int>(a);
}

//---------------------------------------------------------------------------//
//! Convert int to Axis enum value
inline CELER_FUNCTION Axis to_axis(int a)
{
    CELER_EXPECT(a >= 0 && a < 3);
    return static_cast<Axis>(a);
}

//---------------------------------------------------------------------------//
//! Convert Bound enum value to int
CELER_CONSTEXPR_FUNCTION int to_int(Bound b)
{
    return static_cast<int>(b);
}

//---------------------------------------------------------------------------//
//! Get the lowercase name of the axis.
inline constexpr char to_char(Axis ax)
{
    return "xyz\a"[static_cast<int>(ax)];
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
