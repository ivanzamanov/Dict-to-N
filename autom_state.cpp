#include"autom_state.hpp"

// ------------------------
// ------ Autom_State -----
// ------------------------

#define TR_SIZE 256

int Autom_State::transitionCount() const {
  return TR_SIZE;
}

void Autom_State::trAdded() {
  incoming++;
}

void Autom_State::trRemoved() {
  incoming--;
}

int Autom_State::getTr(unsigned int c) const {
  return tr[c];
}

void Autom_State::addTr(unsigned int c, int dest) {
  if(dest == -1) // If this is a delete
    outgoing--;
  else if(tr[c] == -1) // If this is not a replace
    outgoing++;

  tr[c] = dest;
}

void Autom_State::removeTr(unsigned int c) {
  outgoing--;
  tr[c] = -1;
}

void Autom_State::reset() {
  incoming = 0;
  outgoing = 0;
  isDeleted = 0;
  isFinal = 0;
  for(int i=0; i<TR_SIZE; i++)
    tr[i] = -1;
}

int Autom_State::getHash() const {
  int result = 0;
  for(int i=0; i<TR_SIZE; i++) {
    if(tr[i] >= 0) {
      result = (result + tr[i] + i) * TR_SIZE;
    }
  }
  return result < 0 ? -result : result;
}

bool Autom_State::operator==(const Autom_State& other) const {
  if(isFinal != other.isFinal || outgoing != other.outgoing)
    return 0;
  for(int i=0; i<TR_SIZE; i++) {
    if(tr[i] != other.tr[i])
      return 0;
  }
  return 1;
}

// -------------------------------
// ------ TransitionIterator -----
// -------------------------------

void find(const Autom_State& state, int index, int& found) {
  while(index < state.transitionCount() && state.tr[index] < 0)
    index++;
  found = -1 * (index == state.transitionCount()) + index * (index < state.transitionCount());
}

void TransitionIterator::begin() {
  find(state, 0, currentIndex);
  nextIndex = currentIndex;
  currentIndex = -1;
}

bool TransitionIterator::hasNext() {
  return nextIndex >= 0;
}

Transition TransitionIterator::next() {
  int result = nextIndex;
  find(state, nextIndex + 1, nextIndex);
  return Transition(result, state.tr[result]);
}
