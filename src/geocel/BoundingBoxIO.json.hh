//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file geocel/BoundingBoxIO.json.hh
//---------------------------------------------------------------------------//
#pragma once

#include <nlohmann/json.hpp>

#include "BoundingBox.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
// Read a bounding box from a JSON file
template<class T>
void from_json(nlohmann::json const& j, BoundingBox<T>& bbox);

// Write a bounding box to a JSON file
template<class T>
void to_json(nlohmann::json& j, BoundingBox<T> const& bbox);

//---------------------------------------------------------------------------//
}  // namespace celeritas
