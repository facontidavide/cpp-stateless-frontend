#include "state_machine.hpp"

namespace stateless{

const Trigger CompositeState::trigger_start_("start_composite_state");


void CompositeState::onExitImpl()  {
  std::cout << ">>>>>>>>>> ending" << this->name_ << std::endl;
  onExit();
}

void CompositeState::addTransition(const State &src_state, const Trigger &trigger, const State &dest_state)
{
  bool src_state_is_child = false;
  bool dest_state_is_child = false;
  for (const auto& child: children())
  {
    if( *child == src_state)
      src_state_is_child = true;
    else if( *child == dest_state)
      dest_state_is_child = true;
  }
  if( !src_state_is_child){
    throw error(
          "The source state passed as 1st argument of attachTrigger must be a child of this CompositeState");
  }
  if( !dest_state_is_child && dest_state != *this){
    throw error(
          "The destination state passed as 1st argument of attachTrigger must be"
          "a child of this CompositeState or the CompositeState itself if it is a final trigger.");
  }
  sm_.configure(src_state).permit(trigger, dest_state);
}

void CompositeState::addTransition(const Transition &trans)
{
  addTransition( trans.source(), trans.trigger(), trans.destination());
}

void CompositeState::addTransitionToParentState(const State &src_state, const Trigger &trigger)
{
  addTransition(src_state, trigger, *src_state.parent());
}

void CompositeState::addCallbackOnEntry(const State &state, const TransitionCallback &callback)
{
  bool state_is_child = false;

  for (const auto& child: children())
  {
    if( *child == state) state_is_child = true;
  }
  if( !state_is_child){
    throw error("The helper function addCallbackOnEntry shall be used only on a child state");
  }
  sm_.configure(state).on_entry( callback);
}


void CompositeState::addCallbackOnExit(const State &state, const TransitionCallback &callback)
{
  bool state_is_child = false;

  for (const auto& child: children())
  {
    if( *child == state) state_is_child = true;
  }
  if( !state_is_child){
    throw error("The helper function addCallbackOnExit shall be used only on a child state");
  }
  sm_.configure(state).on_exit( callback);
}


CompositeState::CompositeState(StateMachine &sm, const char *name, State *parent_state):
  State(name, parent_state),
  sm_(sm)
{
  if( parent_state_ ){
    sm_.configure(*this)
        .on_entry( std::bind( &CompositeState::onEntryImpl, this) )
        .on_exit( std::bind( &CompositeState::onExitImpl, this) )
        .sub_state_of( *parent_state_ );
  }
  else{
    sm_.configure(*this)
        .on_entry( std::bind( &CompositeState::onEntryImpl, this) )
        .on_exit( std::bind( &CompositeState::onExitImpl, this) );
  }
}


void CompositeState::onEntryImpl()  {
  std::cout << ">>>>>>>>>> starting " << this->name_ ;
  if(parent()) std::cout << " child of " << parent()->name();
  std::cout << std::endl;

  for( auto& child: children())
  {
    sm_.configure(*child).sub_state_of(*this);
  }

  sm_.configure(*this).permit(trigger_start_, initialState() );
  deferredFire( trigger_start_ );

  onEntry();
}

void CompositeState::deferredFire(const Trigger &trigger)
{
  sm_.push_deferred_trigger(trigger);
  if( trigger == getDoneTrigger() || trigger == getFailedTrigger()){
    sm_.push_deferred_trigger(trigger);
  }
}

NamedItem::NamedItem(const char *name){
  static int count = 0;
  char buff[100];
  sprintf(buff, "%03d-%s", count++,name);
  name_ = buff;
}

NamedItem::NamedItem(const NamedItem &other):
  name_(other.name_)
{

}

bool NamedItem::operator <(const NamedItem &other) const{
  return name_< other.name_;
}

bool NamedItem::operator ==(const NamedItem &other) const{
  return name_== other.name_;
}

bool NamedItem::operator !=(const NamedItem &other) const{
  return name_!= other.name_;
}

std::ostream &operator<<(std::ostream &os, const NamedItem &s){
  os << s.name_;
  return os;
}

NamedItem &NamedItem::operator==(const NamedItem &other)
{
  name_ = other.name_;
}

State::State(const char *name, State *parent_state):
  NamedItem(name),
  parent_state_(parent_state)
{
  if( parent_state ) parent_state->children_.push_back( this );
}

State::State(): parent_state_(nullptr) {}

Trigger::Trigger(const char *name):
  NamedItem(name)
{}

}
