#include<cstdlib>
#include<cstring>

#include"autom_state.hpp"

void deallocateStates(Autom_State* ptr, int size) {
  for(int i=0; i<size; i++)
    free(ptr[i].tr);
  free(ptr);
};

void initTransitions(Transition* tr, int count) {
  for(int i=0; i<count; i++) {
    tr[i](-1, -1);
    tr[i].payload = 0;
  }
}

static int initialCap = 1;
void initState(Autom_State* ptr) {
  ptr->isFinal = 0;
  ptr->payload = NON_FINAL_PAYLOAD;
  ptr->tr = (Transition*) malloc(initialCap * sizeof(Transition));
  initTransitions(ptr->tr, initialCap);
  ptr->cap = initialCap;
  ptr->incoming = 0;
  ptr->outgoing = 0;
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
  memcpy(tr, source.tr, cap * sizeof(Transition));
  for(int i = 0; i<cap; i++) {
    int copyDest = tr[i].target;
    if(copyDest >= 0)
      states[copyDest].trAdded();
  }
  outgoing = source.outgoing;
}

Transition Autom_State::getTr(unsigned int c) const {
  short sgc = c;
  for(int i=0; i<cap; i++) {
    if(tr[i].c == sgc)
      return tr[i];
  }
  return Transition(-1);
}

void Autom_State::addTr(const Transition& trans) {
  if(trans.target == -1)// If this is a delete
    outgoing--;
  for(int i=0; i<cap; i++) {
    if(tr[i].c == trans.c || tr[i].c == -1) {
      if(tr[i].target == -1) // If this is not a replace
	outgoing++;
      tr[i] = trans;
      return;
    }
  }
  cap++;
  tr = (Transition*) realloc(tr, cap * sizeof(Transition));
  initTransitions(tr+1, 1);
  tr[cap-1] = trans;
}

void Autom_State::removeTr(const Transition& trans) {
  for(int i=0; i<cap; i++)
    if(tr[i].c == trans.c) {
      tr[i](-1, -1, 0);
      outgoing--;
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
  free(tr);
  initState(this);
}

bool Autom_State::isDeleted() {
  return false;
}

int Autom_State::getHash() const {
  int result = 0;
  TransitionIterator it(*this);
  while(it.hasNext()) {
    Transition tr = it.next();
    result = (result + tr.target + tr.c + tr.payload) * outgoing;
  }
  return result < 0 ? -result : result;
}

bool Autom_State::operator==(const Autom_State& other) const {
  if(isFinal != other.isFinal || outgoing != other.outgoing || payload != other.payload)
    return 0;
  TransitionIterator it1(*this);
  TransitionIterator it2(other);
  while(it1.hasNext() && it2.hasNext()) {
    Transition tr1 = it1.next();
    Transition tr2 = it2.next();
    if(tr1.c != tr2.c || tr1.target != tr2.target || tr1.payload != tr2.payload)
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

Transition& TransitionIterator::next() {
  int result = nextIndex;
  find(state, nextIndex + 1, nextIndex);
  return state.tr[result];
}
