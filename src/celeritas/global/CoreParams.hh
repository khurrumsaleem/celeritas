//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/global/CoreParams.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>

#include "corecel/Assert.hh"
#include "corecel/data/DeviceVector.hh"
#include "corecel/data/ObserverPtr.hh"
#include "corecel/data/ParamsDataInterface.hh"
#include "corecel/random/params/RngParamsFwd.hh"
#include "celeritas/geo/GeoFwd.hh"

#include "ActionInterface.hh"
#include "CoreTrackData.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
class CutoffParams;
class GeoMaterialParams;
class MaterialParams;
class MpiCommunicator;
class OutputRegistry;
class ParticleParams;
class PhysicsParams;
class SimParams;
class SurfaceParams;
class TrackInitParams;
class VolumeParams;
class WentzelOKVIParams;

class ActionRegistry;
class AuxParamsRegistry;

//---------------------------------------------------------------------------//
/*!
 * Global parameters required to run a problem.
 *
 * \todo Applications specify \c tracks_per_stream to build the states, but
 * unit tests currently omit this option.
 */
class CoreParams final : public ParamsDataInterface<CoreParamsData>
{
  public:
    //!@{
    //! \name Type aliases
    using SPConstCoreGeo = std::shared_ptr<CoreGeoParams const>;
    using SPConstMaterial = std::shared_ptr<MaterialParams const>;
    using SPConstGeoMaterial = std::shared_ptr<GeoMaterialParams const>;
    using SPConstParticle = std::shared_ptr<ParticleParams const>;
    using SPConstCutoff = std::shared_ptr<CutoffParams const>;
    using SPConstPhysics = std::shared_ptr<PhysicsParams const>;
    using SPConstRng = std::shared_ptr<RngParams const>;
    using SPConstSim = std::shared_ptr<SimParams const>;
    using SPConstSurface = std::shared_ptr<SurfaceParams const>;
    using SPConstTrackInit = std::shared_ptr<TrackInitParams const>;
    using SPConstVolume = std::shared_ptr<VolumeParams const>;

    using SPConstWentzelOKVI = std::shared_ptr<WentzelOKVIParams const>;

    using SPActionRegistry = std::shared_ptr<ActionRegistry>;
    using SPOutputRegistry = std::shared_ptr<OutputRegistry>;
    using SPAuxRegistry = std::shared_ptr<AuxParamsRegistry>;
    using SPConstMpiCommunicator = std::shared_ptr<MpiCommunicator const>;

    template<MemSpace M>
    using ConstRef = CoreParamsData<Ownership::const_reference, M>;
    template<MemSpace M>
    using ConstPtr = ObserverPtr<ConstRef<M> const, M>;
    //!@}

    struct Input
    {
        SPConstCoreGeo geometry;
        SPConstMaterial material;
        SPConstGeoMaterial geomaterial;
        SPConstParticle particle;
        SPConstCutoff cutoff;
        SPConstPhysics physics;
        SPConstRng rng;
        SPConstSim sim;
        SPConstSurface surface;
        SPConstTrackInit init;
        SPConstVolume volume;
        SPConstWentzelOKVI wentzel;  //!< Optional (TODO: move to EM physics)

        SPActionRegistry action_reg;
        SPOutputRegistry output_reg;
        SPAuxRegistry aux_reg;  //!< Optional, empty default
        SPConstMpiCommunicator mpi_comm;  //!< Optional, world_comm default

        //! Maximum number of simultaneous threads/tasks per process
        StreamId::size_type max_streams{1};

        //! Number of track slots per stream
        StreamId::size_type tracks_per_stream{0};

        //! True if all params are assigned and valid
        explicit operator bool() const
        {
            return geometry && material && geomaterial && particle && cutoff
                   && physics && rng && sim && surface && init && volume
                   && action_reg && output_reg && max_streams;
        }
    };

  public:
    // Construct with all problem data, creating some actions too
    explicit CoreParams(Input inp);

    //!@{
    //! Access shared problem parameter data.
    SPConstCoreGeo const& geometry() const { return input_.geometry; }
    SPConstMaterial const& material() const { return input_.material; }
    SPConstGeoMaterial const& geomaterial() const
    {
        return input_.geomaterial;
    }
    SPConstParticle const& particle() const { return input_.particle; }
    SPConstCutoff const& cutoff() const { return input_.cutoff; }
    SPConstPhysics const& physics() const { return input_.physics; }
    SPConstRng const& rng() const { return input_.rng; }
    SPConstSim const& sim() const { return input_.sim; }
    SPConstSurface const& surface() const { return input_.surface; }
    SPConstTrackInit const& init() const { return input_.init; }
    SPConstVolume const& volume() const { return input_.volume; }
    SPConstWentzelOKVI const& wentzel() const { return input_.wentzel; }
    SPActionRegistry const& action_reg() const { return input_.action_reg; }
    SPOutputRegistry const& output_reg() const { return input_.output_reg; }
    SPAuxRegistry const& aux_reg() const { return input_.aux_reg; }
    SPConstMpiCommunicator const& mpi_comm() const { return input_.mpi_comm; }
    //!@}

    //! Access data on the host
    HostRef const& host_ref() const final { return host_ref_; }

    //! Access data on the device
    DeviceRef const& device_ref() const final { return device_ref_; }

    // Access host pointers to core data
    using ParamsDataInterface<CoreParamsData>::ref;

    // Access a native pointer to properties in the native memory space
    template<MemSpace M>
    inline ConstPtr<M> ptr() const;

    //! Maximum number of streams
    size_type max_streams() const { return input_.max_streams; }

    //! Number of track slots per stream
    size_type tracks_per_stream() const { return input_.tracks_per_stream; }

  private:
    Input input_;
    HostRef host_ref_;
    DeviceRef device_ref_;

    // Copy of DeviceRef in device memory
    DeviceVector<DeviceRef> device_ref_vec_;
};

//---------------------------------------------------------------------------//
/*!
 * Access a native pointer to a NativeCRef.
 *
 * This way, CUDA kernels only need to copy a pointer in the kernel arguments,
 * rather than the entire (rather large) DeviceRef object.
 */
template<MemSpace M>
auto CoreParams::ptr() const -> ConstPtr<M>
{
    if constexpr (M == MemSpace::host)
    {
        return make_observer(&host_ref_);
    }
    else
    {
        CELER_ENSURE(!device_ref_vec_.empty());
        return make_observer(device_ref_vec_);
    }
#if CELER_CUDACC_BUGGY_IF_CONSTEXPR
    CELER_ASSERT_UNREACHABLE();
#endif
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
