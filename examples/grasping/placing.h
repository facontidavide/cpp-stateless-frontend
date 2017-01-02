#ifndef PLACING_HPP
#define PLACING_HPP

#include "state_machine.hpp"

using namespace stateless;



class PlacingAction: public CompositeState
{
public:

  State state_approaching;
  State state_dropping;
  State state_retracting;
  State state_action_failed;

  static Trigger trigger_approached;
  static Trigger trigger_drop_succesful;
  static Trigger trigger_placing_failure;
  static Trigger trigger_placing_done;

  PlacingAction(StateMachine& sm, const char* state_name, State* parent_state)
    : CompositeState(sm, state_name, parent_state),
      state_approaching("approaching",this),
      state_dropping("dropping",this),
      state_retracting("retracting",this),
      state_action_failed("action_failed",this)

  {
    using namespace std::placeholders;

    addCallbackOnEntry(state_approaching,
                       std::bind(&PlacingAction::executeApproaching, this, _1));

    addTransition(state_approaching, trigger_approached, state_dropping);
    addTransition(state_approaching, trigger_placing_failure, state_action_failed);

    addCallbackOnEntry(state_dropping,
                       std::bind(&PlacingAction::executeDropping, this, _1));

    addTransition(state_dropping, trigger_drop_succesful, state_retracting);
    addTransition(state_dropping, trigger_placing_failure, state_action_failed);

    addCallbackOnEntry(state_retracting,
                       std::bind(&PlacingAction::executeRetracting, this, _1));

    addTransitionToParentState(state_retracting, trigger_placing_failure);
    addTransitionToParentState(state_retracting, trigger_placing_done);
  }

  virtual const  Trigger& getDoneTrigger()  const override { return trigger_placing_done; }
  virtual const  Trigger& getFailedTrigger() const override { return trigger_placing_failure; }
  virtual const  State&   initialState() const override  { return state_approaching; }

private:

  void executeApproaching(const Transition& tr){
    std::cout << "Placing executeApproaching" << std::endl;
    deferredFire( trigger_approached );
  }

  void executeDropping(const Transition& tr){
    std::cout << "Placing executeDropping" << std::endl;
    deferredFire( trigger_drop_succesful );
  }
  void executeRetracting(const Transition& tr){
    std::cout << "Placing executeRetracting" << std::endl;
    deferredFire( trigger_placing_done );
  }

};

Trigger PlacingAction::trigger_approached("approached");
Trigger PlacingAction::trigger_drop_succesful("drop_succesful");
Trigger PlacingAction::trigger_placing_failure("Placing_failure");
Trigger PlacingAction::trigger_placing_done("Placing_done");


#endif // PLACING_HPP
