#include<cstdlib>
#include<cstring>

#include"autom_state.hpp"

void deallocateStates(Autom_State* ptr) {
  free(ptr);
};

void initTransitions(Transition* tr, int count) {
  for(int i=0; i<count; i++)
    tr[i](-1, -1);
}

static int initialCap = 4;
void initState(Autom_State* ptr) {
  ptr->isFinal = 0;
  ptr->isDeleted = 0;
  ptr->tr = (Transition*) malloc(initialCap * sizeof(Transition));
  initTransitions(ptr->tr, initialCap);
  ptr->cap = initialCap;
};

Autom_State* allocateStates(int count) {
  Autom_State* result = (Autom_State*) malloc(count * sizeof(Autom_State));
  for(int i=0; i<count; i++)
    initState(result + i);
  return result;
};

Autom_State* reallocateStates(Autom_State* ptr, int oldSize, int newSize) {
  Autom_State* result = (Autom_State*) realloc(ptr, newSize * sizeof(Autom_State));
  for(int i=oldSize; i<newSize; i++)
    initState(result + i);
  return result;
};

void find(const Autom_State& state, int index, int& found) {
  while(index < state.transitionCount() && state.tr[index].c < 0)
    index++;
  found = -1 * (index == state.transitionCount()) + index * (index < state.transitionCount());
}

// ------------------------
// ------ Autom_State -----
// ------------------------

void Autom_State::copyTransitions(const Autom_State& source, Autom_State* states) {
  cap = source.cap;
  tr = (Transition*) realloc(tr, cap * sizeof(Transition));
  memcpy(tr, source.tr, cap * sizeof(int));
  for(int i = 0; i<cap; i++) {
    int copyDest = tr[i].target;
    if(copyDest >= 0)
      states[copyDest].trAdded();
  }
}

int Autom_State::getTr(unsigned int c) const {
  int sgc = c;
  for(int i=0; i<cap; i++) {
    if(tr[i].c == sgc)
      return tr[i].target;
  }
  return -1;
}

void Autom_State::addTr(unsigned int c, int dest) {
  if(dest == -1)// If this is a delete
    outgoing--;
  int sgc = c;
  for(int i=0; i<cap; i++) {
    if(tr[i].c == sgc || tr[i].c == -1) {
      if(tr[i].target == -1) // If this is not a replace
	outgoing++;
      tr[i](c, dest);
      return;
    }
  }
  // expand
  cap+=2;
  tr = (Transition*) realloc(tr, cap * sizeof(Transition));
  initTransitions(tr + cap - 2, 2);
  tr[cap-2](c, dest);
}

void Autom_State::removeTr(unsigned int c) {
  int sgnc = c;
  for(int i=0; i<cap; i++)
    if(tr[i].c == sgnc) {
      tr[i](-1, -1);
      break;
    }
}

int Autom_State::transitionCount() const {
  return cap;
}

void Autom_State::trAdded() {
  incoming++;
}

void Autom_State::trRemoved() {
  incoming--;
}

void Autom_State::reset() {
  incoming = 0;
  outgoing = 0;
  isDeleted = 0;
  isFinal = 0;
  for(int i=0; i<cap; i++)
    tr[i](-1, -1);
}

int Autom_State::getHash() const {
  int result = 0;
  TransitionIterator it(*this);
  while(it.hasNext()) {
    Transition tr = it.next();
    result = (result + tr.target + tr.c) * outgoing;
  }
  return result < 0 ? -result : result;
}

bool Autom_State::operator==(const Autom_State& other) const {
  if(isFinal != other.isFinal || outgoing != other.outgoing)
    return 0;
  TransitionIterator it1(*this);
  TransitionIterator it2(other);
  while(it1.hasNext() && it2.hasNext()) {
    Transition tr1 = it1.next();
    Transition tr2 = it2.next();
    if(tr1.c != tr2.c || tr1.target != tr2.target)
      return false;
  }
  return it1.hasNext() == it2.hasNext();
}

// -------------------------------
// ------ TransitionIterator -----
// -------------------------------

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
  return state.tr[result];
}
