/**
 * Copyright 2013 Matt Mason
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Example inspired by
// http://www.drdobbs.com/cpp/state-machine-design-in-c/184401236#.

#include "state_machine.hpp"
#include "picking.h"
#include "placing.h"

using namespace stateless;
using namespace std::placeholders;

class PickAndPlaceAction: public CompositeState
{
public:

  State state_detecting_part;
  PickingAction state_picking;
  PlacingAction state_placing;

  static Trigger trigger_part_found;
  static Trigger trigger_pp_failure;
  static Trigger trigger_pp_done;

  PickAndPlaceAction(StateMachine& sm, const char* state_name, State* parent_state)
    : CompositeState(sm, state_name, parent_state),
      state_detecting_part("detecting_part", this),
      state_picking(sm, "picking", this),
      state_placing(sm,"placing", this)
  {
    using namespace std::placeholders;

    sm.configure(state_detecting_part)
        .on_entry( std::bind(&PickAndPlaceAction::executeDetectingPart, this, _1));

    addTransition(state_detecting_part, trigger_part_found, state_picking);

    sm.configure(state_picking)
        .permit(state_picking.getDoneTrigger(), state_placing);


    addTransitionToParentState(state_placing, state_placing.getDoneTrigger()) ;
    addTransitionToParentState(state_placing, state_placing.getFailedTrigger()) ;

  }

  virtual const  Trigger& getDoneTrigger() const override   { return trigger_pp_done; }
  virtual const  Trigger& getFailedTrigger() const override { return trigger_pp_failure; }

private:
  virtual const  State&   initialState() const override  { return state_detecting_part; }

  void executeDetectingPart(const Transition& tr){
    std::cout << "PickAndPlace executeDetectingPart" << std::endl;
    deferredFire( trigger_part_found );
  }


};

Trigger PickAndPlaceAction::trigger_part_found("part_found");
Trigger PickAndPlaceAction::trigger_pp_failure("P&P_failure");
Trigger PickAndPlaceAction::trigger_pp_done("P&P_done");




int main(int argc, char* argv[])
{
  State initial_state("idle", nullptr);
  State final_state("DONE", nullptr);

  Trigger initial_trigger("start");


  StateMachine sm( initial_state );

  // Register a callback for state transitions (the default does nothing).
  sm.on_transition([](const Transition& t)
  {
    std::cout << "transition from [" << t.source() << "] to ["
              << t.destination() << "] via trigger [" << t.trigger() << "]"
              << std::endl;
  });

  // Override the default behaviour of throwing when a trigger is unhandled.
  sm.on_unhandled_trigger([](const State& s, const Trigger& t)
  {
    std::cout << "ignore unhandled trigger [" << t << "] in state [" << s
              << "]" << std::endl;
  });

  //--------------------------------------------------------

  PickAndPlaceAction pp(sm, "pick_and_place", nullptr);

  sm.configure( initial_state )
    .permit(initial_trigger, pp );

  sm.configure( pp )
    .permit( pp.getDoneTrigger(), final_state );

  sm.fire( initial_trigger );

  while( sm.pop_deferred_trigger() )
  {
     std::cout << sm << std::endl;
  }


  return EXIT_SUCCESS;
}

