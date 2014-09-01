#include<cstdlib>
#include<cstring>

#include"stack.hpp"
#include"autom_state.hpp"

// -------------------------------
// --- Allocation and Pooling ----
// -------------------------------

IFDEBUG(static int allocTrCalls = 0);
IFDEBUG(static int allocTrHits = 0);
IFDEBUG(static int allocTrMiss = 0);

IFDEBUG(static int reallocTrCalls = 0);
IFDEBUG(static int reallocTrHits = 0);
IFDEBUG(static int reallocTrMiss = 0);

IFDEBUG(static int deallocTrCalls = 0);

#define BLOCK_SIZE 1024 * 1024

AutomAllocator::AutomAllocator() {
  Transition* newPool = (Transition*) malloc(BLOCK_SIZE * sizeof(Transition));
  allocPool.push(AllocEntry(newPool, 0));
}

AutomAllocator::~AutomAllocator() {
  while(!allocPool.isEmpty()) {
    free(allocPool.pop().ptr);
  }
}

void AutomAllocator::printPools() {
  for(int i=0; i<MAX_TR; i++) {
    if(!pools[i].isEmpty())
      printf("Pool %d = %d\n", i, pools[i].size());
  }
  LOG(allocTrCalls);
  LOG(allocTrHits);
  LOG(allocTrMiss);
  LOG(reallocTrCalls);
  LOG(reallocTrHits);
  LOG(reallocTrMiss);
  LOG(deallocTrCalls);
}

Transition* AutomAllocator::allocateTransitions(int cap) {
  IFDEBUG(allocTrCalls++);
  Transition* result;
  if(!pools[cap].isEmpty()) {
    IFDEBUG(allocTrHits++);
    result = pools[cap].pop();
  } else {
    IFDEBUG(allocTrMiss++);
    AllocEntry& e = allocPool.peek();
    if(e.next + cap < BLOCK_SIZE) {
      result = (e.ptr + e.next);
      e.next += cap;
    } else {
      Transition* newPool = (Transition*) malloc(BLOCK_SIZE * sizeof(Transition));
      allocPool.push(AllocEntry(newPool, cap));
      pools[BLOCK_SIZE - e.next].push(e.ptr);
      result = newPool;
    }
  }
  return result;
}

void AutomAllocator::deallocateTransitions(Transition* tr, int cap) {
  IFDEBUG(deallocTrCalls++);
  pools[cap].push(tr);
}

Transition* AutomAllocator::reallocateTransitions(Transition* tr, int oldCap, int newCap) {
  IFDEBUG(reallocTrCalls++);
  Transition* result;
  if(!pools[newCap].isEmpty()) {
    IFDEBUG(reallocTrHits++);
    result = pools[newCap].pop();
    deallocateTransitions(tr, oldCap);
  } else {
    IFDEBUG(reallocTrMiss++);
    // result = (Transition*) realloc(tr, newCap * sizeof(Transition));
    result = allocateTransitions(newCap);
  }
  // TODO: use memcpy
  for(int i=0; i<oldCap; i++)
    result[i] = tr[i];
  return result;
}

void initTransitions(Transition* tr, int count) {
  for(int i=0; i<count; i++) {
    tr[i](-1, -1);
    tr[i].payload = 0;
  }
}

static int initialCap = 1;
void initState(AutomAllocator& alloc, Autom_State* ptr) {
  ptr->isFinal = 0;
  ptr->payload = NON_FINAL_PAYLOAD;
  ptr->tr = alloc.allocateTransitions(initialCap);
  initTransitions(ptr->tr, initialCap);
  ptr->cap = initialCap;
  ptr->incoming = 0;
  ptr->outgoing = 0;
};

Autom_State* AutomAllocator::allocateStates(int count) {
  Autom_State* result = (Autom_State*) malloc(count * sizeof(Autom_State));
  return result;
};

void AutomAllocator::deallocateStates(Autom_State* ptr, int size) {
  for(int i=0; i<size; i++) {
    if(!ptr[i].isDeleted())
      deallocateTransitions(ptr[i].tr, ptr[i].cap);
  }
  free(ptr);
};

Autom_State* AutomAllocator::reallocateStates(Autom_State* ptr, int oldSize, int newSize) {
  Autom_State* result = (Autom_State*) realloc(ptr, newSize * sizeof(Autom_State));
  return result;
};

void AutomAllocator::copyTransitions(const Autom_State& source, Autom_State& dest, Autom_State* states) {
  dest.cap = source.cap;
  deallocateTransitions(dest.tr, dest.cap);
  dest.tr = allocateTransitions(dest.cap);
  memcpy(dest.tr, source.tr, dest.cap * sizeof(Transition));
  for(int i = 0; i<dest.cap; i++) {
    int copyDest = dest.tr[i].target;
    if(copyDest >= 0)
      states[copyDest].trAdded();
  }
  dest.outgoing = source.outgoing;
}

// ------------------------
// --------- MISC ---------
// ------------------------

void find(const Autom_State& state, int index, int& found) {
  while(index < state.transitionCount() && state.tr[index].c < 0)
    index++;
  found = -1 * (index == state.transitionCount()) + index * (index < state.transitionCount());
}

// ------------------------
// ------ Autom_State -----
// ------------------------

void Autom_State::destroy(AutomAllocator& alloc) {
  alloc.deallocateTransitions(tr, cap);
}

void Autom_State::init(AutomAllocator& alloc) {
  initState(alloc, this);
}

Transition Autom_State::getTr(unsigned int c) const {
  short sgc = c;
  for(int i=0; i<cap; i++) {
    if(tr[i].c == sgc)
      return tr[i];
  }
  return Transition(-1);
}

void Autom_State::addTr(AutomAllocator& alloc, const Transition& trans) {
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
  tr = alloc.reallocateTransitions(tr, cap-1, cap);
  //  tr = (Transition*) realloc(tr, cap * sizeof(Transition));
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

void Autom_State::reset(AutomAllocator& alloc) {
  alloc.deallocateTransitions(tr, cap);
  initState(alloc, this);
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
