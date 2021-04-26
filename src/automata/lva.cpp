#include "lva.hpp"

#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <list>
#include <cassert>
#include <map>

#include "automata/lvastate.hpp"
#include "factories/factories.hpp"
#include "parser/parser.hpp"

namespace rematch {

LogicalVA::LogicalVA()
    : init_state_(new LVAState()),
      v_factory_(std::make_shared<VariableFactory>()),
      f_factory_(std::make_shared<FilterFactory>()) {
  init_state_->setInitial(true);
  states.push_back(init_state_);
}

LogicalVA::LogicalVA(uint code) {
  init_state_ = new_state();
  init_state_->setInitial(true);
  LVAState* fstate = new_final_state();

  init_state_->addFilter(code, fstate);
}

void LogicalVA::set_factories(std::shared_ptr<VariableFactory> v,
										          std::shared_ptr<FilterFactory> f) {
  v_factory_ = v;
  f_factory_ = f;
}

void LogicalVA::adapt_capture_jumping() {
  std::vector<LVAState*> stack;
  LVAState *reached_state;


  for(auto &state: states) {
    stack.clear();

    for(auto &capture: state->captures) {
      stack.push_back(capture->next);
      state->tempMark = false;
    }

    while(!stack.empty()) {
      reached_state = stack.back();
      stack.pop_back();

      reached_state->tempMark = true;

      if(!reached_state->filters.empty() || !reached_state->epsilons.empty() || reached_state->isFinal)
        state->addEpsilon(reached_state);
      for(auto &capture: reached_state->captures) {
        if(!capture->next->tempMark)
          stack.push_back(capture->next);
      }
    }
  }

  for(auto &state: states)
    state->captures.clear();
}

void LogicalVA::cat(LogicalVA &a2) {
  /* Concatenates an LogicalVA a2 to the current LogicalVA (inplace) */

  // Adds eps transitions from final states to a2 init LVAState
  for(std::size_t i=0;i<finalStates.size();i++){
    finalStates[i]->addEpsilon(a2.init_state_);
    finalStates[i]->setFinal(false);
  }

  // Add a2 states to states list
  states.insert(states.end(), a2.states.begin(), a2.states.end());

  // Set a2 final states as new final states
  finalStates.clear();
  finalStates = a2.finalStates;
}

void LogicalVA::alter(LogicalVA &a2) {
  /* Extends the LogicalVA so it can alternate between itself and an
     LogicalVA a2 */

  // Creates a new init LVAState and connects it to the old init and a2's init
  LVAState* newinitState = new LVAState();
  newinitState->addEpsilon(init_state_);
  newinitState->addEpsilon(a2.init_state_);

  init_state_ = newinitState;

  states.push_back(init_state_);
  // Add a2 final states to final states list
  finalStates.insert(finalStates.end(),
    a2.finalStates.begin(), a2.finalStates.end());

  // Add a2 states to states list
  states.insert(states.end(), a2.states.begin(), a2.states.end());
}

void LogicalVA::kleene() {
  /* Extends the LogicalVA for kleene closure (0 or more times) */

  if(states.size() == 2 && init_state_->filters.size() == 1) {
    if(init_state_->filters.front()->next->isFinal) {
      for(auto it=states.begin(); it != states.end(); it++) {
        if(!(*it)->isInit) {
          states.erase(it); break;
        }
      }

      auto fcode = init_state_->filters.front()->code;
      init_state_->filters.clear();

      init_state_->addFilter(fcode, init_state_);
      finalStates.clear();
      finalStates.push_back(init_state_);
      init_state_->setFinal(true);
      return;
    }
  }

  // Connect final states to new init LVAState
  for(std::size_t i=0;i<finalStates.size();i++){
    finalStates[i]->addEpsilon(init_state_);
    finalStates[i]->setFinal(false);
  }

  // Set new init as the final LVAState
  finalStates.clear();
  finalStates.push_back(init_state_);
  init_state_->setFinal(true);
}

void LogicalVA::strict_kleene() {
  /* Extends the LogicalVA for strict kleene closure (1 or more times) */

  // Connect final states to init LVAState
  for(std::size_t i=0;i<finalStates.size();i++){
    finalStates[i]->addEpsilon(init_state_);
  }
}

void LogicalVA :: optional() {
  /* Extends the LogicalVA for optional closure (0 or 1 time) */

  // Set new init as a final LVAState
  if (!init_state_->isFinal)
  {
    finalStates.push_back(init_state_);
    init_state_->setFinal(true);
  }
}

void LogicalVA::assign(std::bitset<32> open_code, std::bitset<32> close_code) {
  /* Extends the LogicalVA so it can assign its pattern to a variable */

  // Create new states
  LVAState* openLVAState = new_state();
  LVAState* closeLVAState = new_state();

  // Connect new open LVAState with init LVAState
  openLVAState->addCapture(open_code, init_state_);

  // Set open LVAState as new init LVAState
  init_state_ = openLVAState;

  // Connect final states with new close LVAState
  for(std::size_t i=0;i<finalStates.size();i++){
    finalStates[i]->addCapture(close_code, closeLVAState);
    finalStates[i]->setFinal(false);
  }

  // Set close LVAState as the only final LVAState
  finalStates.clear();
  finalStates.push_back(closeLVAState);
  closeLVAState->setFinal(true);
}

std::string LogicalVA :: pprint() {
  /* Gives a codification for the LogicalVA that can be used to visualize it
     at https://puc-iic2223.github.io . Basically it uses Breath-First Search
     to get every labeled transition in the LogicalVA with the unique ids for
     each LVAState */


  // Declarations
  std::stringstream ss, cond;
  LVAState *current;
  int cid, nid;  // cid: current LVAState id; nid : next LVAState id
  std::bitset<32> S;



  // Typical set construction for keeping visited states
  std::unordered_set<unsigned int> visited;

  // Use of list to implement a FIFO queue
  std::list<LVAState*> queue;

  // Start on the init LVAState
  visited.insert(init_state_->id);
  queue.push_back(init_state_);



  // Start BFS
  while(!queue.empty()) {
    current = queue.front();
    queue.pop_front();
    cid = current->id;

    // For every epsilon transition
    for (auto &epsilon: current->epsilons) {
      nid = epsilon->next->id;

      ss << "t " << cid << " eps " << nid << '\n';

      // If not visited enqueue and add to visited
      if (visited.find(nid) == visited.end()) {
        visited.insert(nid);
        queue.push_back(epsilon->next);
      }
    }

    // For every capture transition
    for (auto &capture: current->captures) {
      S = capture->code;

      nid = capture->next->id;

      ss << "t " << cid << " " << v_factory_->getVarUtil(S) << " " << nid << '\n';

      // If not visited enqueue and add to visited
      if (visited.find(nid) == visited.end()) {
        visited.insert(nid);
        queue.push_back(capture->next);
      }
    }

    // For every filter transition
    for (auto &filter: current->filters) {
      nid = filter->next->id;
      S = filter->code;

      // TODO: Get the correct transition name

      ss << "t " << cid << ' ' << f_factory_->getFilter(filter->code).print() << ' ' << nid << '\n';

      // If not visited enqueue and add to visited
      if (visited.find(nid) == visited.end()) {
        visited.insert(nid);
        queue.push_back(filter->next);
      }
    }
  }

  // Code final states
  for (size_t i = 0; i < finalStates.size(); ++i) {
    if(finalStates[i]->isFinal) {
      ss << "f " << finalStates[i]->id << '\n';
    }
  }

  // Code initial LVAState
  ss << "i " << init_state_->id;

  return ss.str();
}

LVAState* LogicalVA::new_state() {
  LVAState *nstate = new LVAState();
  states.push_back(nstate);

  return nstate;
}

LVAState* LogicalVA::new_final_state() {
  LVAState *nstate = new LVAState();
  states.push_back(nstate);

  finalStates.push_back(nstate);
  nstate->setFinal(true);

  return nstate;
}

} // end namespace rematch