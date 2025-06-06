//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file corecel/sys/detail/KernelTraitsImpl.hh
//---------------------------------------------------------------------------//
#pragma once

#include <type_traits>

namespace celeritas
{
namespace detail
{
//---------------------------------------------------------------------------//
//! Checks if type T has a `max_block_size` static member data
template<typename T, typename = void>
struct HasMaxBlockSize : std::false_type
{
};

template<typename T>
struct HasMaxBlockSize<T, std::void_t<decltype(T::max_block_size)>>
    : std::true_type
{
};

template<typename T>
inline constexpr bool has_max_block_size_v = HasMaxBlockSize<T>::value;

//---------------------------------------------------------------------------//
//! Checks if type T has a `min_warps_per_eu` static member data
template<typename T, typename = void>
struct HasMinWarpsPerEU : std::false_type
{
};

template<typename T>
struct HasMinWarpsPerEU<T, std::void_t<decltype(T::min_warps_per_eu)>>
    : std::true_type
{
};

template<typename T>
inline constexpr bool has_min_warps_per_eu_v = HasMinWarpsPerEU<T>::value;

//---------------------------------------------------------------------------//
//! Checks if type T declared an `Applier` member type
template<typename T, typename = void>
struct HasApplier : std::false_type
{
};

template<typename T>
struct HasApplier<T, std::void_t<typename T::Applier>> : std::true_type
{
};

template<typename T>
inline constexpr bool has_applier_v = HasApplier<T>::value;

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
