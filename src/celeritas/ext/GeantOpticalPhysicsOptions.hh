//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/ext/GeantOpticalPhysicsOptions.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Types.hh"
#include "celeritas/Types.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
//! Cherenkov process options
struct CherenkovPhysicsOptions
{
    //! Enable the process
    bool enable{true};
    //! Enable generation of Cherenkov photons
    bool stack_photons{true};
    //! Track generated photons before parent
    bool track_secondaries_first{true};
    //! Maximum number of photons that can be generated before limiting step
    int max_photons{100};
    //! Maximum percentage change in particle \f$\beta\f$  before limiting step
    double max_beta_change{10.0};

    //! True if the process is activated
    explicit operator bool() const { return enable; }
};

//! Equality operator, mainly for test harness
constexpr bool
operator==(CherenkovPhysicsOptions const& a, CherenkovPhysicsOptions const& b)
{
    // clang-format off
    return a.enable == b.enable
           && a.stack_photons == b.stack_photons
           && a.track_secondaries_first == b.track_secondaries_first
           && a.max_photons == b.max_photons
           && a.max_beta_change == b.max_beta_change;
    // clang-format on
}

//---------------------------------------------------------------------------//
//! Scintillation process options
struct ScintillationPhysicsOptions
{
    //! Enable the process
    bool enable{true};

    //! Enable generation of scintillation photons
    bool stack_photons{true};
    //! Track generated photons before parent
    bool track_secondaries_first{true};
    //! Use per-particle yield and time constants for photon generation
    bool by_particle_type{false};
    //! Use material properties for sampling photon generation time
    bool finite_rise_time{false};
    //! Attach scintillation interaction information to generated photon
    bool track_info{false};

    //! True if the process is activated
    explicit operator bool() const { return enable; }
};

//! Equality operator, mainly for test harness
constexpr bool operator==(ScintillationPhysicsOptions const& a,
                          ScintillationPhysicsOptions const& b)
{
    // clang-format off
    return a.enable == b.enable
           && a.stack_photons == b.stack_photons
           && a.track_secondaries_first == b.track_secondaries_first
           && a.by_particle_type == b.by_particle_type
           && a.finite_rise_time == b.finite_rise_time
           && a.track_info == b.track_info;
    // clang-format on
}

//---------------------------------------------------------------------------//
//! Optical wavelength shifting process options
struct WavelengthShiftingOptions
{
    //! Enable the process
    bool enable{true};
    //! Select a model for sampling reemission time
    WlsTimeProfile time_profile{WlsTimeProfile::delta};

    //! True if the process is activated
    explicit operator bool() const { return enable; }
};

//! Equality operator, mainly for test harness
constexpr bool operator==(WavelengthShiftingOptions const& a,
                          WavelengthShiftingOptions const& b)
{
    return a.enable == b.enable && a.time_profile == b.time_profile;
}

//---------------------------------------------------------------------------//
//! Optical boundary process options
struct BoundaryPhysicsOptions
{
    //! Enable the process
    bool enable{true};
    //! Invoke Geant4 SD at post step point if photon deposits energy
    bool invoke_sd{false};

    //! True if the process is activated
    explicit operator bool() const { return enable; }
};

//! Equality operator, mainly for test harness
constexpr bool
operator==(BoundaryPhysicsOptions const& a, BoundaryPhysicsOptions const& b)
{
    return a.enable == b.enable && a.invoke_sd == b.invoke_sd;
}

//---------------------------------------------------------------------------//
/*!
 * Construction options for Geant optical physics.
 *
 * These options attempt to default to our closest match to \c
 * G4OpticalPhysics from Geant4 10.5 onwards.
 *
 * \todo When we require C++20, use `friend bool operator==(...) = default;`
 * instead of manually writing the equality operators
 */
struct GeantOpticalPhysicsOptions
{
    //!@{
    //! \name Optical photon creation physics

    //! Cherenkov radiation options
    CherenkovPhysicsOptions cherenkov;
    //! Scintillation options
    ScintillationPhysicsOptions scintillation;
    //!@}

    //!@{
    //! \name Optical photon physics

    //! Enable wavelength shifting
    WavelengthShiftingOptions wavelength_shifting;
    //! Enable second wavelength shifting
    WavelengthShiftingOptions wavelength_shifting2;
    //! Enable boundary effects
    BoundaryPhysicsOptions boundary;
    //! Enable absorption
    bool absorption{true};
    //! Enable Rayleigh scattering
    bool rayleigh_scattering{true};
    //! Enable Mie scattering
    bool mie_scattering{true};
    //!@}

    //! Print detailed Geant4 output
    bool verbose{false};

    //! True if any process is activated
    explicit operator bool() const
    {
        return cherenkov || scintillation || wavelength_shifting
               || wavelength_shifting2 || boundary || absorption
               || rayleigh_scattering || mie_scattering;
    }

    //! Return instance with all processes deactivated
    static GeantOpticalPhysicsOptions deactivated()
    {
        GeantOpticalPhysicsOptions opts;
        opts.cherenkov.enable = false;
        opts.scintillation.enable = false;
        opts.wavelength_shifting.enable = false;
        opts.wavelength_shifting2.enable = false;
        opts.boundary.enable = false;
        opts.absorption = false;
        opts.rayleigh_scattering = false;
        opts.mie_scattering = false;
        return opts;
    }
};

//! Equality operator, mainly for test harness
constexpr bool operator==(GeantOpticalPhysicsOptions const& a,
                          GeantOpticalPhysicsOptions const& b)
{
    // clang-format off
    return a.cherenkov == b.cherenkov
           && a.scintillation == b.scintillation
           && a.wavelength_shifting == b.wavelength_shifting
           && a.wavelength_shifting2 == b.wavelength_shifting2
           && a.boundary == b.boundary
           && a.absorption == b.absorption
           && a.rayleigh_scattering == b.rayleigh_scattering
           && a.mie_scattering == b.mie_scattering
           && a.verbose == b.verbose;
    // clang-format on
}

//---------------------------------------------------------------------------//
// FREE FUNCTIONS
//---------------------------------------------------------------------------//

char const* to_cstring(WlsTimeProfile value);

//---------------------------------------------------------------------------//
}  // namespace celeritas
