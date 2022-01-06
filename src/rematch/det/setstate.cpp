#include <vector>
#include <bitset>
#include <set>

#include "setstate.hpp"
#include "automata/nfa/state.hpp"
#include "automata/nfa/eva.hpp"

namespace rematch {

SetState :: SetState(ExtendedVA const &eVA, std::set<State*> l):
	subset(l), bitstring(eVA.size()) {
	/* Construct the bitstring inmediately */
	isFinal = false;
	isSuperFinal = false;
	isNonEmpty = false;
	for(auto &state: subset) {
		bitstring.set(state->id,true);

		// Check if the subset is a final subset
		if(state->accepting()) isFinal = true;
		// if(state->isSuperFinal) isSuperFinal = true;
	}

	if(bitstring.count()) isNonEmpty = true;
}


std::ostream & operator<<(std::ostream &os, SetState const &ss) {
  if(ss.subset.empty()) return os << "{}";
  os << "{";
  for(auto &state: ss.subset) {
  	if(state == *ss.subset.begin()) {
  		os << state->id;
  	}
  	else {
  		os << "," << state->id;
  	}
  }
  os << "}";
  return os;
}

} // end namespace rematch

