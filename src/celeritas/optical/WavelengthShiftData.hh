//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/WavelengthShiftData.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Macros.hh"
#include "corecel/Types.hh"
#include "corecel/data/Collection.hh"
#include "corecel/grid/NonuniformGridData.hh"
#include "celeritas/Quantities.hh"
#include "celeritas/Types.hh"

namespace celeritas
{
namespace optical
{
//---------------------------------------------------------------------------//
/*!
 * Input data for sampling WLS optical photons.
 */
struct WlsDistributionData
{
    size_type num_photons{};  //!< Sampled number of photons to generate
    units::MevEnergy energy;
    real_type time{};  //!< Post-step time
    Real3 position{};
    OptMatId material;

    //! Check whether the data are assigned
    explicit CELER_FUNCTION operator bool() const
    {
        return num_photons > 0 && energy > zero_quantity() && material;
    }
};

//---------------------------------------------------------------------------//
/*!
 * Material dependent scalar property of wavelength shift (WLS).
 */
struct WlsMaterialRecord
{
    real_type mean_num_photons{};  //!< Mean number of reemitted photons
    real_type time_constant{};  //!< Time delay of WLS [time]

    //! Whether all data are assigned and valid
    explicit CELER_FUNCTION operator bool() const
    {
        return mean_num_photons > 0 && time_constant > 0;
    }
};

//---------------------------------------------------------------------------//
/*!
 * Wavelength shift data
 */
template<Ownership W, MemSpace M>
struct WavelengthShiftData
{
    template<class T>
    using Items = Collection<T, W, M>;
    template<class T>
    using OpticalMaterialItems = Collection<T, W, M, OptMatId>;

    //// MEMBER DATA ////

    OpticalMaterialItems<WlsMaterialRecord> wls_record;

    // Cumulative probability of emission as a function of energy
    OpticalMaterialItems<NonuniformGridRecord> energy_cdf;

    // Time profile model
    WlsTimeProfile time_profile{WlsTimeProfile::size_};

    // Backend data
    Items<real_type> reals;

    //// MEMBER FUNCTIONS ////

    //! Whether all data are assigned and valid
    explicit CELER_FUNCTION operator bool() const
    {
        return !wls_record.empty() && !energy_cdf.empty()
               && time_profile != WlsTimeProfile::size_;
    }

    //! Assign from another set of data
    template<Ownership W2, MemSpace M2>
    WavelengthShiftData& operator=(WavelengthShiftData<W2, M2> const& other)
    {
        CELER_EXPECT(other);
        wls_record = other.wls_record;
        energy_cdf = other.energy_cdf;
        time_profile = other.time_profile;
        reals = other.reals;
        return *this;
    }
};

//---------------------------------------------------------------------------//
}  // namespace optical
}  // namespace celeritas
