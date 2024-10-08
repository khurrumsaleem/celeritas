#----------------------------------*-CMake-*----------------------------------#
# Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

set(SOURCES)
set(PRIVATE_DEPS nlohmann_json::nlohmann_json)
set(PUBLIC_DEPS Celeritas::corecel)

#-----------------------------------------------------------------------------#
# Main code
#-----------------------------------------------------------------------------#

list(APPEND SOURCES
  BoundingBoxIO.json.cc
  GeoParamsOutput.cc
  rasterize/Color.cc
  rasterize/Image.cc
  rasterize/ImageIO.json.cc
)

#-----------------------------------------------------------------------------#
# Configuration-dependent code
#-----------------------------------------------------------------------------#

if(CELERITAS_USE_PNG)
  celeritas_add_object_library(geocel_png
    rasterize/ImageWriter.libpng.cc
  )
  celeritas_target_link_libraries(geocel_png PRIVATE PNG::PNG Celeritas::corecel)

  list(APPEND SOURCES $<TARGET_OBJECTS:geocel_png>)
  list(APPEND PRIVATE_DEPS geocel_png)
endif()

if(CELERITAS_USE_VecGeom AND CELERITAS_USE_Geant4)
  celeritas_add_object_library(geocel_g4vg
    g4vg/Converter.cc
    g4vg/LogicalVolumeConverter.cc
    g4vg/SolidConverter.cc
  )

  celeritas_get_g4libs(_cg4vg_libs geometry)
  list(APPEND _cg4vg_libs Celeritas::corecel VecGeom::vecgeom)
  celeritas_target_link_libraries(geocel_g4vg PRIVATE ${_cg4vg_libs})

  list(APPEND SOURCES $<TARGET_OBJECTS:geocel_g4vg>)
  list(APPEND PRIVATE_DEPS geocel_g4vg)
endif()

if(CELERITAS_USE_Geant4)
  celeritas_add_object_library(geocel_geant4
    GeantUtils.cc
    GeantGeoUtils.cc
    ScopedGeantExceptionHandler.cc
    ScopedGeantLogger.cc
    g4/GeantGeoParams.cc
    g4/RaytraceImager.cc
    g4/detail/GeantGeoNavCollection.cc
  )

  celeritas_get_g4libs(_cg4_libs global geometry materials persistency intercoms)
  list(APPEND _cg4_libs Celeritas::corecel XercesC::XercesC)
  celeritas_target_link_libraries(geocel_geant4 PRIVATE ${_cg4_libs})

  list(APPEND SOURCES $<TARGET_OBJECTS:geocel_geant4>)
  list(APPEND PRIVATE_DEPS geocel_geant4)
endif()

if(CELERITAS_USE_VecGeom)
  list(APPEND SOURCES
    vg/VecgeomParams.cc
    vg/VecgeomParamsOutput.cc
    vg/detail/VecgeomNavCollection.cc
    vg/RaytraceImager.cc
  )
  if(VecGeom_GDML_FOUND)
    list(APPEND PRIVATE_DEPS VecGeom::vgdml)
  endif()
  if(VecGeom_CUDA_FOUND AND VecGeom_SURF_FOUND)
    # Special routines needed for surface
    list(APPEND SOURCES
      vg/VecgeomParams.surface.cu
    )
  endif()
  if(CELERITAS_USE_CUDA)
    # This needs to be public because its might be needed
    # to resolve the symbols generate by the `nvcc -dlink` of
    # one of the executable.
    list(APPEND PUBLIC_DEPS VecGeom::vecgeom)
    list(APPEND SOURCES
      vg/RaytraceImager.cu
    )
  elseif(VecGeom_CUDA_FOUND)
    # VecGeom is built with CUDA but Celeritas is not
    list(APPEND PRIVATE_DEPS VecGeom::vecgeomcuda)
  endif()
endif()

#-----------------------------------------------------------------------------#
# Create library
#-----------------------------------------------------------------------------#

celeritas_add_library(geocel ${SOURCES})

celeritas_target_link_libraries(geocel
  PRIVATE ${PRIVATE_DEPS}
  PUBLIC ${PUBLIC_DEPS}
)

#-----------------------------------------------------------------------------#


