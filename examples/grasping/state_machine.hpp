#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include <stateless++/state_machine.hpp>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace stateless{


class NamedItem{
public:
  NamedItem(const char* name);
  virtual ~NamedItem() = default;

  NamedItem( const NamedItem& other);

  NamedItem& operator==( const NamedItem& other);

  NamedItem(){}

  const std::string name() const { return name_; }

  bool operator <(const NamedItem& other) const;

  friend std::ostream& operator<<(std::ostream& os, const NamedItem& s);

  bool operator ==(const NamedItem& other) const;

  bool operator !=(const NamedItem& other) const;

protected:
  std::string name_;
};


class State: public NamedItem
{
public:

  State(const char* name, State* parent_state);
  State();
  virtual ~State() = default;

  const State*parent() const { return parent_state_; }
  const std::vector<const State*>& children() const { return children_; }

protected:

  const State* parent_state_;
  std::vector<const State*> children_;

};

class Trigger: public NamedItem
{
public:
  Trigger(const char* name);
  Trigger() = default;
};

typedef state_machine<State, Trigger> StateMachine;
typedef StateMachine::TTransition Transition;
typedef std::function<void(const Transition&)> TransitionCallback;


class CompositeState: public State{
public:

  CompositeState(StateMachine& sm, const char* name, State* parent_state);

  virtual ~CompositeState() = default;

  StateMachine&  getStateMachine() { return sm_; }

  void addTransition(const State& src_state, const Trigger& trigger, const State& dest_state);
  void addTransition(const Transition& trans);
  void addTransitionToParentState(const State& src_state, const Trigger& trigger);

  void addCallbackOnEntry(const State& state, const TransitionCallback& callback);
  void addCallbackOnExit(const State& state, const TransitionCallback& callback);

  virtual void onEntry() {}
  virtual void onExit() {}

  virtual const Trigger& getDoneTrigger()  const = 0;
  virtual const Trigger& getFailedTrigger() const = 0;

  void deferredFire(const Trigger& trigger);

protected:

  void onEntryImpl();
  void onExitImpl();

  virtual const State&   initialState() const = 0;
  const static Trigger trigger_start_;
  StateMachine& sm_;
};


template<> inline void print_state<State>(std::ostream& os, const State& s)
{ os << s; }

template<> inline void print_trigger<Trigger>(std::ostream& os, const Trigger& t)
{ os << t; }


}

#endif // STATE_MACHINE_HPP
