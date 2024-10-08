//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/global/detail/ActionSequence.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "corecel/Types.hh"

#include "../ActionInterface.hh"
#include "../CoreTrackDataFwd.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
class ActionRegistry;
class CoreParams;
class StatusChecker;

namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Sequence of step actions to invoke as part of a single step.
 *
 * TODO accessors here are used by diagnostic output from celer-sim etc.;
 * perhaps make this public or add a diagnostic output for it?
 */
template<class P, template<MemSpace M> class S>
class ActionSequence
{
  public:
    //!@{
    //! \name Type aliases
    using BeginRunActionT = BeginRunActionInterface<P, S>;
    using StepActionT = StepActionInterface<P, S>;
    using SPBegin = std::shared_ptr<BeginRunActionT>;
    using SPConstStepAction = std::shared_ptr<StepActionT const>;
    using VecBeginAction = std::vector<SPBegin>;
    using VecStepAction = std::vector<SPConstStepAction>;
    using VecDouble = std::vector<double>;
    //!@}

    //! Construction/execution options
    struct Options
    {
        bool action_times{false};  //!< Call DeviceSynchronize and add timer
    };

  public:
    // Construct from an action registry and sequence options
    ActionSequence(ActionRegistry const&, Options options);

    //// INVOCATION ////

    // Launch all actions with the given memory space.
    template<MemSpace M>
    void begin_run(P const& params, S<M>& state);

    // Launch all actions with the given memory space.
    template<MemSpace M>
    void step(P const&, S<M>& state);

    //// ACCESSORS ////

    //! Whether synchronization is taking place
    bool action_times() const { return options_.action_times; }

    //! Get the set of beginning-of-run actions
    VecBeginAction const& begin_run_actions() const { return begin_run_; }

    //! Get the ordered vector of actions in the sequence
    VecStepAction const& actions() const { return actions_; }

    //! Get the corresponding accumulated time, if 'sync' or host called
    VecDouble const& accum_time() const { return accum_time_; }

  private:
    Options options_;
    VecBeginAction begin_run_;
    VecStepAction actions_;
    VecDouble accum_time_;
    std::shared_ptr<StatusChecker const> status_checker_;
};

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
