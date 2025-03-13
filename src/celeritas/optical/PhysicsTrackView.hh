//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/PhysicsTrackView.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Macros.hh"
#include "celeritas/Quantities.hh"
#include "celeritas/Types.hh"
#include "celeritas/grid/NonuniformGridCalculator.hh"

#include "PhysicsData.hh"
#include "Types.hh"

namespace celeritas
{
namespace optical
{
//---------------------------------------------------------------------------//
/*!
 * Optical physics data for a track.
 *
 * The physics track data provides an interface for data and operations common
 * to most optical models.
 */
class PhysicsTrackView
{
  public:
    //!@{
    //! \name Type aliases
    using PhysicsParamsRef = NativeCRef<PhysicsParamsData>;
    using PhysicsStateRef = NativeRef<PhysicsStateData>;
    using Energy = units::MevEnergy;
    //!@}

    //! Data for initializing a physics track
    struct Initializer
    {
    };

  public:
    // Construct from params, state, and material ID for a given track
    inline CELER_FUNCTION PhysicsTrackView(PhysicsParamsRef const&,
                                           PhysicsStateRef const&,
                                           OpticalMaterialId,
                                           TrackSlotId);

    // Initialize the physics for the track
    inline CELER_FUNCTION PhysicsTrackView& operator=(Initializer const&);

    //// Discrete interaction mean free path ////

    // Reset the currently calculated MFP
    inline CELER_FUNCTION void reset_interaction_mfp();

    // Change the interaction MFP
    inline CELER_FUNCTION void interaction_mfp(real_type);

    // Get the current MFP
    inline CELER_FUNCTION real_type interaction_mfp() const;

    // Whether there's a currently calculated MFP
    inline CELER_FUNCTION bool has_interaction_mfp() const;

    //// Step-Local (non-persistent) track data ////

    // Set total cross section of the step
    inline CELER_FUNCTION void macro_xs(real_type xs);

    // Retrieve total cross section of the step
    inline CELER_FUNCTION real_type macro_xs() const;

    //// Model-Action mappings ////

    // Number of optical models
    inline CELER_FUNCTION ModelId::size_type num_models() const;

    // Map a model ID to an action ID
    inline CELER_FUNCTION ActionId model_to_action(ModelId) const;

    // Map an action ID to a model ID
    inline CELER_FUNCTION ModelId action_to_model(ActionId) const;

    // ID of the discrete action
    inline CELER_FUNCTION ActionId discrete_action() const;

    //// Mean free path grids ////

    // Get MFP grid ID for the given model
    inline CELER_FUNCTION ValueGridId mfp_grid(ModelId) const;

    // Calculate the MFP for the given model and energy
    inline CELER_FUNCTION real_type calc_mfp(ModelId, Energy) const;

  private:
    PhysicsParamsRef const& params_;
    PhysicsStateRef const& states_;
    OpticalMaterialId const opt_material_;
    TrackSlotId const track_id_;
};

//---------------------------------------------------------------------------//
// INLINE FUNCTIONS
//---------------------------------------------------------------------------//
/*!
 * Construct from params, state, and material ID for a given track.
 */
CELER_FUNCTION
PhysicsTrackView::PhysicsTrackView(PhysicsParamsRef const& params,
                                   PhysicsStateRef const& states,
                                   OpticalMaterialId opt_mat,
                                   TrackSlotId track_id)
    : params_(params)
    , states_(states)
    , opt_material_(opt_mat)
    , track_id_(track_id)
{
    CELER_EXPECT(track_id_ < states_.size());
    CELER_EXPECT(opt_mat < params_.scalars.num_materials);
}

//---------------------------------------------------------------------------//
/*!
 * Initialize the physics for the given track.
 */
CELER_FUNCTION PhysicsTrackView& PhysicsTrackView::operator=(Initializer const&)
{
    this->reset_interaction_mfp();
    return *this;
}

//---------------------------------------------------------------------------//
/*!
 * Reset the currently calculated interaction MFP.
 */
CELER_FUNCTION void PhysicsTrackView::reset_interaction_mfp()
{
    states_.interaction_mfp[track_id_] = 0;
}

//---------------------------------------------------------------------------//
/*!
 * Retrieve the interaction mean free path.
 */
CELER_FUNCTION void PhysicsTrackView::interaction_mfp(real_type mfp)
{
    states_.interaction_mfp[track_id_] = mfp;
}

//---------------------------------------------------------------------------//
/*!
 * Retrieve the interaction mean free path.
 */
CELER_FUNCTION real_type PhysicsTrackView::interaction_mfp() const
{
    return states_.interaction_mfp[track_id_];
}

//---------------------------------------------------------------------------//
/*!
 * Whether there's a calculated interaction MFP.
 */
CELER_FUNCTION bool PhysicsTrackView::has_interaction_mfp() const
{
    return states_.interaction_mfp[track_id_] > 0;
}

//---------------------------------------------------------------------------//
/*!
 * Retrieve the number of optical models.
 */
CELER_FUNCTION ModelId::size_type PhysicsTrackView::num_models() const
{
    return params_.scalars.num_models;
}

//---------------------------------------------------------------------------//
/*!
 * Convert a model ID to an action ID.
 */
CELER_FUNCTION ActionId PhysicsTrackView::model_to_action(ModelId mid) const
{
    CELER_EXPECT(mid < this->num_models());
    return ActionId{mid.get() + params_.scalars.model_to_action};
}

//---------------------------------------------------------------------------//
/*!
 * Convert an action ID to a model ID.
 */
CELER_FUNCTION ModelId PhysicsTrackView::action_to_model(ActionId aid) const
{
    if (!aid)
        return ModelId{};

    // Rely on unsigned rollover if action ID is less than the first model
    ModelId::size_type result = aid.unchecked_get()
                                - params_.scalars.model_to_action;

    if (result >= this->num_models())
        return ModelId{};

    return ModelId{result};
}

//---------------------------------------------------------------------------//
/*!
 * Get the action ID for the discrete action.
 */
CELER_FUNCTION ActionId PhysicsTrackView::discrete_action() const
{
    return params_.scalars.discrete_action();
}

//---------------------------------------------------------------------------//
/*!
 * Get the MFP grid ID for the given model.
 *
 * The grid corresponds to the optical material this track view was
 * constructed with.
 */
CELER_FUNCTION ValueGridId PhysicsTrackView::mfp_grid(ModelId model) const
{
    CELER_EXPECT(model < this->num_models());

    ValueGridId grid_id{opt_material_.get()
                        + model.get() * params_.scalars.num_materials};

    CELER_ENSURE(grid_id < params_.grids.size());
    return grid_id;
}

//---------------------------------------------------------------------------//
/*!
 * Calculate the MFP for the given model and energy.
 *
 * Energy is interpolated using \c NonuniformGridCalculator for the model's
 * MFP grid.
 */
CELER_FUNCTION real_type PhysicsTrackView::calc_mfp(ModelId model,
                                                    Energy energy) const
{
    NonuniformGridCalculator calc{params_.grids[this->mfp_grid(model)], params_.reals};
    real_type result = calc(value_as<Energy>(energy));
    CELER_ENSURE(result > 0);
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Retrieve total cross section for this step.
 */
CELER_FUNCTION real_type PhysicsTrackView::macro_xs() const
{
    return states_.macro_xs[track_id_];
}

//---------------------------------------------------------------------------//
/*!
 * Set total cross section for this step.
 */
CELER_FUNCTION void PhysicsTrackView::macro_xs(real_type xs)
{
    states_.macro_xs[track_id_] = xs;
}

//---------------------------------------------------------------------------//
}  // namespace optical
}  // namespace celeritas
