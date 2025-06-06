//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/neutron/model/detail/NuclearZoneBuilder.hh
//---------------------------------------------------------------------------//
#pragma once

#include <cmath>
#include <numeric>
#include <vector>

#include "corecel/cont/Range.hh"
#include "corecel/cont/Span.hh"
#include "corecel/math/Integrator.hh"
#include "celeritas/Types.hh"
#include "celeritas/mat/IsotopeView.hh"
#include "celeritas/neutron/data/NeutronInelasticData.hh"
#include "celeritas/phys/AtomicNumber.hh"

#include "../CascadeOptions.hh"

namespace celeritas
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Construct NuclearZoneData for NeutronInelasticModel.
 */
class NuclearZoneBuilder
{
  public:
    //!@{
    //! \name Type aliases
    using AtomicMassNumber = AtomicNumber;
    using MevMass = units::MevMass;
    using Energy = units::MevEnergy;
    using ComponentVec = std::vector<ZoneComponent>;
    using Data = HostVal<NuclearZoneData>;
    //!@}

  public:
    // Construct with cascade options and shared data
    inline NuclearZoneBuilder(CascadeOptions const& options,
                              NeutronInelasticScalars const& scalars,
                              Data* data);

    // Construct nuclear zone data for a target (isotope)
    inline void operator()(IsotopeView const& target);

  private:
    //// TYPES ////

    struct ZoneDensity
    {
        real_type radius;
        real_type integral;
    };
    using VecZoneDensity = std::vector<ZoneDensity>;

    //// DATA ////

    // Cascade model configurations and nuclear structure parameters
    CascadeOptions const& options_;

    real_type skin_depth_;
    MevMass neutron_mass_;
    MevMass proton_mass_;

    CollectionBuilder<ZoneComponent> components_;
    CollectionBuilder<NuclearZones, MemSpace::host, IsotopeId> zones_;

    //// HELPER FUNCTIONS ////

    // Calculate components of nuclear zone properties
    inline ComponentVec calc_zone_components(IsotopeView const& target) const;

    // Calculate zone densities for lightweight nuclei (A < 5)
    VecZoneDensity calc_zones_light(AtomicMassNumber a) const;

    // Calculate zone densities for small nuclei (5 <= A < 12)
    VecZoneDensity calc_zones_small(AtomicMassNumber a) const;

    // Calculate zone densities for heavier nuclei (A >= 12)
    VecZoneDensity calc_zones_heavy(AtomicMassNumber a) const;

    // Calculate the nuclear radius
    inline real_type calc_nuclear_radius(AtomicMassNumber a) const;
};

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Construct with cascade options and data.
 */
NuclearZoneBuilder::NuclearZoneBuilder(CascadeOptions const& options,
                                       NeutronInelasticScalars const& scalars,
                                       Data* data)
    : options_(options)
    , skin_depth_(0.611207 * options.radius_scale)
    , neutron_mass_(scalars.neutron_mass)
    , proton_mass_(scalars.proton_mass)
    , components_(&data->components)
    , zones_(&data->zones)
{
    CELER_EXPECT(options_);
    CELER_EXPECT(data);
}

//---------------------------------------------------------------------------//
/*!
 * Construct nuclear zone data for a single isotope.
 */
void NuclearZoneBuilder::operator()(IsotopeView const& target)
{
    auto comp = this->calc_zone_components(target);

    NuclearZones nucl_zones;
    nucl_zones.zones = components_.insert_back(comp.begin(), comp.end());
    zones_.push_back(nucl_zones);
}

//---------------------------------------------------------------------------//
/*!
 * Calculate components of nuclear zone data.
 *
 * The nuclear zone radius, volume,
 * density, Fermi momentum and potential function as in G4NucleiModel and as
 * documented in section 24.2.3 of the Geant4 Physics Reference (release 11.2).
 *
 */
auto NuclearZoneBuilder::calc_zone_components(IsotopeView const& target) const
    -> ComponentVec
{
    using A = AtomicMassNumber;

    // Calculate nuclear radius
    A const a = target.atomic_mass_number();

    // Calculate per-zone radius and potential integral
    VecZoneDensity zone_dens;
    if (a < A{5})
    {
        zone_dens = this->calc_zones_light(a);
    }
    else if (a < A{12})
    {
        zone_dens = this->calc_zones_small(a);
    }
    else
    {
        zone_dens = this->calc_zones_heavy(a);
    }
    CELER_ASSERT(!zone_dens.empty());

    // Fill the differential nuclear volume by each zone
    constexpr real_type four_thirds_pi = 4 * constants::pi / real_type{3};

    ComponentVec components(zone_dens.size());
    real_type prev_volume{0};
    for (auto i : range(components.size()))
    {
        real_type volume = four_thirds_pi * ipow<3>(zone_dens[i].radius);

        // Save radius and differential volume
        components[i].radius = zone_dens[i].radius;
        components[i].volume = volume - prev_volume;
        prev_volume = volume;
    }

    // Fill the nuclear zone density, fermi momentum, and potential
    int num_protons = target.atomic_number().get();
    int num_nucleons[] = {num_protons, a.get() - num_protons};
    real_type mass[] = {proton_mass_.value(), neutron_mass_.value()};
    real_type dm[] = {target.proton_loss_energy().value(),
                      target.neutron_loss_energy().value()};
    static_assert(std::size(mass) == ZoneComponent::NucleonArray::size());
    static_assert(std::size(dm) == std::size(mass));

    real_type const total_integral
        = std::accumulate(zone_dens.begin(),
                          zone_dens.end(),
                          real_type{0},
                          [](real_type sum, ZoneDensity const& zone) {
                              return sum + zone.integral;
                          });

    for (auto i : range(components.size()))
    {
        for (auto ptype : range(ZoneComponent::NucleonArray::size()))
        {
            components[i].density[ptype]
                = static_cast<real_type>(num_nucleons[ptype])
                  * zone_dens[i].integral
                  / (total_integral * components[i].volume);
            components[i].fermi_mom[ptype]
                = options_.fermi_scale
                  * std::cbrt(components[i].density[ptype]);
            components[i].potential[ptype]
                = ipow<2>(components[i].fermi_mom[ptype]) / (2 * mass[ptype])
                  + dm[ptype];
        }
    }
    return components;
}

//---------------------------------------------------------------------------//
/*!
 * Lightweight nuclei are treated as simple balls.
 */
auto NuclearZoneBuilder::calc_zones_light(AtomicMassNumber a) const
    -> VecZoneDensity
{
    CELER_EXPECT(a <= AtomicMassNumber{4});
    ZoneDensity result;
    result.radius = options_.radius_small
                    * (a == AtomicMassNumber{4} ? options_.radius_alpha : 1);
    result.integral = 1;
    return {result};
}

//---------------------------------------------------------------------------//
/*!
 * Small nuclei have a three-zone gaussian potential.
 */
auto NuclearZoneBuilder::calc_zones_small(AtomicMassNumber a) const
    -> VecZoneDensity
{
    real_type const nuclear_radius = this->calc_nuclear_radius(a);
    real_type gauss_radius = std::sqrt(
        ipow<2>(nuclear_radius) * (1 - 1 / static_cast<real_type>(a.get()))
        + real_type{6.4});

    Integrator integrate_gauss{
        [](real_type r) { return ipow<2>(r) * std::exp(-ipow<2>(r)); }};

    // Precompute y = sqrt(-log(alpha)) where alpha[3] = {0.7, 0.3, 0.01}
    real_type const y[] = {0.597223, 1.09726, 2.14597};
    VecZoneDensity result(std::size(y));

    real_type ymin = 0;
    for (auto i : range(result.size()))
    {
        result[i].radius = gauss_radius * y[i];
        result[i].integral = ipow<3>(gauss_radius)
                             * integrate_gauss(ymin, y[i]);
        ymin = y[i];
    }

    CELER_ENSURE(result.size() == 3);
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Heavy nuclei have a three- or six-zone Woods-Saxon potential.
 *
 * The Woods-Saxon potential, \f$ V(r) \f$,
 *
 * \f[
     V(r) = frac{V_{o}}{1 + e^{\frac{r - R}{a}}}
   \f]
 * is integrated numerically over the volume from \f$ r_{min} \f$ to \f$
 * r_{rmax} \f$, where \f$ V_{o}, R, a\f$ are the potential well depth, nuclear
 * radius, and surface thickness (skin depth), respectively.
 */
auto NuclearZoneBuilder::calc_zones_heavy(AtomicMassNumber a) const
    -> VecZoneDensity
{
    real_type const nuclear_radius = this->calc_nuclear_radius(a);
    real_type const skin_ratio = nuclear_radius / skin_depth_;
    real_type const skin_decay = std::exp(-skin_ratio);
    Integrator integrate_ws{[ws_shift = 2 * skin_ratio](real_type r) {
        return r * (r + ws_shift) / (1 + std::exp(r));
    }};

    Span<real_type const> alpha;
    if (a < AtomicMassNumber{100})
    {
        static real_type const alpha_i[] = {0.7, 0.3, 0.01};
        alpha = make_span(alpha_i);
    }
    else
    {
        static real_type const alpha_h[] = {0.9, 0.6, 0.4, 0.2, 0.1, 0.05};
        alpha = make_span(alpha_h);
    }

    VecZoneDensity result(alpha.size());
    real_type ymin = -skin_ratio;
    for (auto i : range(result.size()))
    {
        real_type y = std::log((1 + skin_decay) / alpha[i] - 1);
        result[i].radius = nuclear_radius + skin_depth_ * y;

        result[i].integral
            = ipow<3>(skin_depth_)
              * (integrate_ws(ymin, y)
                 + ipow<2>(skin_ratio)
                       * std::log((1 + std::exp(-ymin)) / (1 + std::exp(-y))));
        ymin = y;
    }

    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Calculate the nuclear radius (R) computed from the atomic mass number (A).
 *
 * For \f$ A > 4 \f$, the nuclear radius with two parameters takes the form,
 * \f[
     R = [ 1.16 * A^{1/3} - 1.3456 / A^{1/3} ] \cdot R_{scale}
   \f]
 * where \f$ R_{scale} \f$ is a configurable parameter in [femtometer], while
 * \f$ R = 1.2 A^{1/3} \cdot R_{scale} \f$ (default) with a single parameter.
 */
real_type NuclearZoneBuilder::calc_nuclear_radius(AtomicMassNumber a) const
{
    CELER_EXPECT(a > AtomicMassNumber{4});

    // Nuclear radius computed from A
    real_type cbrt_a = std::cbrt(static_cast<real_type>(a.get()));
    real_type par_a = (options_.use_two_params ? 1.16 : 1.2);
    real_type par_b = (options_.use_two_params ? -1.3456 : 0);
    return options_.radius_scale * (par_a * cbrt_a + par_b / cbrt_a);
}

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
