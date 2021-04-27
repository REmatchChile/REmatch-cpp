#include "state.hpp"

#include <list>
#include <set>
#include <bitset>

namespace rematch {

LVACapture :: LVACapture(State* from, std::bitset<32> coding, State* next):
    from(from), next(next), code(coding) {}

bool LVACapture :: operator==(const LVACapture &rhs) const {
  return (code == rhs.code && next == rhs.next && from == rhs.from);
}

bool LVACapture :: operator<(const LVACapture &rhs) const {
  /* Simple ordering for being able to construct an std::set<LVACapture> */
  if(code.to_ulong() != rhs.code.to_ulong()) {
    return code.to_ulong() < rhs.code.to_ulong();
  }

  else if(next != rhs.next) {
    return next < rhs.next;
  }

  return from < rhs.from;
}

State::State()
  : tempMark(false), colorMark('w'), visitedBy(0), isFinal(false),
    isInit(false), isSuperFinal(false) {
  id = ID++;
}

State::State(const State& s)
  : tempMark(false),
    colorMark('w'),
    visitedBy(0),
    isFinal(s.isFinal),
    isInit(s.isInit),
    isSuperFinal(s.isSuperFinal) {
  id = ID++;
}

bool State::operator==(const State &rhs) const { return id == rhs.id;}


State* State::nextLVAState(unsigned int code) {
  for(auto &capture: captures) {
    if (capture->code == code) {
      return capture->next;
    }
  }

  return nullptr;
}

void State::setFinal(bool b) {
  isFinal = b;
}

void State::setInitial(bool b) {
  isInit = b;
}

void State::addEpsilon(State* next) {epsilons.push_back(std::make_shared<LVAEpsilon>(next));}

void State::addCapture(std::bitset<32> code, State* next) {
  for(auto const &capture: this->captures) {
    if (capture->code == code && capture->next == next)
      return;
  }

  auto sp = std::make_shared<LVACapture>(this, code, next);
  captures.push_back(sp);
  next->incidentCaptures.push_back(sp);
}

void State::addFilter(unsigned int code, State* next) {
  for(auto const& filter: this->filters)
    if(filter->code == code && filter->next == next) return;

  auto sp = std::make_shared<LVAFilter>(this, code, next);
  filters.push_back(sp);
  next->incidentFilters.push_back(sp);
}

unsigned int State::ID = 0;


} // end namespace rematch
