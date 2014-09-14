#include<climits>

#ifndef __AUTOM_STATE_H__
#define __AUTOM_STATE_H__

#define NON_FINAL_PAYLOAD INT_MAX
#define MAX_TR 256
#ifdef DEBUG
#define LOG(var)				\
  printf(#var);					\
  printf(" = %d", var);				\
  printf("\n");
#define IFDEBUG(stmt) stmt
#else
#define LOG(var)
#define IFDEBUG(stmt)
#endif

struct AutomAllocator;
struct Transition;
struct Autom_State;
class TransitionIterator;

struct Transition {
  Transition(int c, int target, unsigned int payload)
    : target(target),payload(payload),c(c) { };
  Transition(int c, int target)
    : target(target),payload(0),c(c) { };
  Transition(int c)
    : target(-1),payload(0),c(c) { };
  int target;
  unsigned int payload;
  char c;
  void operator()(int c, int target, unsigned int payload = 0) {
    this->c = c;
    this->target = target;
    this->payload = payload;
  }
};

struct Autom_State {
  Transition* tr;
  unsigned int incoming;
  unsigned char cap;
  unsigned char outgoing;
  unsigned int payload;
  bool isFinal;
  bool deleted;

  void init(AutomAllocator& alloc);
  void destroy(AutomAllocator& alloc);
  bool isDeleted();
  int transitionCount() const;
  void trAdded();
  void trRemoved();
  Transition getTr(unsigned int c) const;
  void addTr(AutomAllocator& alloc, const Transition& trans);
  void removeTr(const Transition& trans);
  int getHash() const;
  bool operator==(const Autom_State& other) const;
};

class TransitionIterator {
public:
  TransitionIterator(const Autom_State& state):state(state) { begin(); }
  void begin();
  Transition& next();
  bool hasNext();

private:
  const Autom_State& state;
  int currentIndex;
  int nextIndex;
};

struct AllocEntry {
  AllocEntry(Transition* ptr, int next):
    ptr(ptr), next(next) { };
  Transition* ptr;
  int next;
};

struct AutomAllocator {
  AutomAllocator();
  ~AutomAllocator();
  Autom_State* allocateStates(int count);
  void deallocateStates(Autom_State* ptr, int size);
  Autom_State* reallocateStates(Autom_State* ptr, int oldSize, int newSize);

  void copyTransitions(const Autom_State& source, Autom_State& dest, Autom_State* allStates);

  Transition* allocateTransitions(int cap);
  void deallocateTransitions(Transition* ptr, int size);
  Transition* reallocateTransitions(Transition* ptr, int oldCap, int newCap);

  void printPools();

  typedef Stack<Transition*> TrPool;
  TrPool pools[MAX_TR];
  typedef Stack<AllocEntry> AllocPool;
  AllocPool allocPool;
};

#endif
