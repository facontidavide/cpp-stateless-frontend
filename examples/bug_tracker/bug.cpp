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

#include "bug.hpp"

#include <iostream>

namespace bug_tracker_example
{

using namespace stateless;
using namespace std::placeholders;

Bug::Bug(const std::string& title)
  : state_(State::open)
  , title_(title)
  , assignee_()
  , state_machine_(std::bind(&Bug::get_state, this), std::bind(&Bug::set_state, this, _1))
  , assign_trigger_()
  , resolve_trigger_()
{
  assign_trigger_ = state_machine_.set_trigger_parameters<std::string>(Trigger::assign);
  resolve_trigger_ = state_machine_.set_trigger_parameters<std::string>(Trigger::resolve);

  state_machine_.configure(State::open)
    .permit(Trigger::assign, State::assigned);

  state_machine_.configure(State::assigned)
    .sub_state_of(State::open)
    .on_entry_from(assign_trigger_, std::bind(&Bug::on_assigned, this, _1, _2))
    .permit_reentry(Trigger::assign)
    .permit(Trigger::resolve, State::resolved)
    .permit(Trigger::close, State::closed)
    .permit(Trigger::defer, State::deferred)
    .on_exit(std::bind(&Bug::on_deassigned, this));

  state_machine_.configure(State::deferred)
    .on_entry([=](const TTransition& t){ assignee_.reset(); })
    .permit(Trigger::assign, State::assigned);
    
  state_machine_.configure(State::resolved)
    .on_entry<std::string>(std::bind(&Bug::on_resolved, this, _1, _2))
    .permit(Trigger::close, State::closed)
    .permit(Trigger::open, State::open);
  
  state_machine_.configure(State::closed)
    .permit(Trigger::open, State::open);
}

void Bug::close()
{
  state_machine_.fire(Trigger::close);
}

void Bug::assign(const std::string& assignee)
{
  state_machine_.fire(assign_trigger_, assignee);
}

bool Bug::can_assign() const
{
  return state_machine_.can_fire(Trigger::assign);
}

void Bug::defer()
{
  state_machine_.fire(Trigger::defer);
}

void Bug::resolve(const std::string& assignee)
{
  state_machine_.fire(resolve_trigger_, assignee);
}

void Bug::open()
{
  state_machine_.fire(Trigger::open);
}

void Bug::on_assigned(const Bug::TTransition& transition, const std::string& assignee)
{
  if (assignee_ != nullptr && assignee != *assignee_)
  {
    send_email_to_assignee("Don't forget to help the new guy.");
  }
  assignee_ = std::make_shared<std::string>(assignee);
  send_email_to_assignee("You own it.");
}

void Bug::on_resolved(const Bug::TTransition& transition, const std::string& assignee)
{
  assignee_ = std::make_shared<std::string>(assignee);
  send_email_to_assignee("It's fixed and ready for test.");
}

void Bug::on_deassigned()
{
  send_email_to_assignee("You're off the hook.");
}

void Bug::send_email_to_assignee(const std::string& message)
{
  std::cout << "To: " << *assignee_ << " Re: " << title_ << std::endl
    << "--" << std::endl << message << std::endl;
}

const Bug::State& Bug::get_state() const
{
  return state_;
}

void Bug::set_state(const Bug::State& new_state)
{
  state_ = new_state;
}

}
