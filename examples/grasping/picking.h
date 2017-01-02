#ifndef PICKING_HPP
#define PICKING_HPP

#include "state_machine.hpp"

using namespace stateless;



class PickingAction: public CompositeState
{
public:

  State state_approaching;
  State state_grasping;
  State state_retracting;

  static Trigger trigger_approached;
  static Trigger trigger_grasp_succesful;
  static Trigger trigger_picking_failure;
  static Trigger trigger_picking_done;

  PickingAction(StateMachine& sm, const char* state_name, State* parent_state)
    : CompositeState(sm, state_name, parent_state),
      state_approaching("approaching", this),
      state_grasping("grasping", this),
      state_retracting("retracting", this)
  {
    using namespace std::placeholders;

    addCallbackOnEntry(state_approaching, std::bind(&PickingAction::executeApproaching, this, _1));
    addCallbackOnExit( state_approaching, std::bind(&PickingAction::doneApproaching, this, _1));

    addTransition(state_approaching, trigger_approached,   state_grasping);
    addTransition(state_approaching, trigger_picking_failure, *this);

    addCallbackOnEntry(state_grasping, std::bind(&PickingAction::executeGrasping, this, _1));
    addCallbackOnExit( state_grasping, std::bind(&PickingAction::doneGrasping, this, _1));

    addTransition(state_grasping, trigger_grasp_succesful,   state_retracting);
    addTransition(state_grasping, trigger_picking_failure, *this);

    addCallbackOnEntry(state_retracting, std::bind(&PickingAction::executeRetracting, this, _1));
    addCallbackOnExit( state_retracting, std::bind(&PickingAction::doneRetracting, this, _1));

    addTransitionToParentState(state_retracting, trigger_picking_done);
    addTransitionToParentState(state_retracting, trigger_picking_failure);
  }

  virtual const Trigger& getDoneTrigger()  const override{ return trigger_picking_done; }
  virtual const Trigger& getFailedTrigger() const override{ return trigger_picking_failure; }

private:
  virtual const State&   initialState() const override { return state_approaching; }

  void executeApproaching(const Transition& tr){
    std::cout << "Picking executeApproaching" << std::endl;
    deferredFire( trigger_approached );
  }

  void executeGrasping(const Transition& tr){
    std::cout << "Picking executeGrasping" << std::endl;
    deferredFire( trigger_grasp_succesful );
  }
  void executeRetracting(const Transition& tr){
    std::cout << "Picking executeRetracting" << std::endl;
    deferredFire( trigger_picking_done );
  }

  void doneApproaching(const Transition& tr){
    std::cout << "Picking doneApproaching" << std::endl;
  }

  void doneGrasping(const Transition& tr){
    std::cout << "Picking doneGrasping" << std::endl;
  }
  void doneRetracting(const Transition& tr){
    std::cout << "Picking doneRetracting" << std::endl;
  }
};


Trigger PickingAction::trigger_approached("approached");
Trigger PickingAction::trigger_grasp_succesful("grasp_succesful");
Trigger PickingAction::trigger_picking_failure("picking_failure");
Trigger PickingAction::trigger_picking_done("picking_done");


#endif // PICKING_HPP
