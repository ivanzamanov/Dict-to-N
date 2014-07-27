#ifndef __AUTOM_STATE_H__
#define __AUTOM_STATE_H__

struct Transition {
  Transition(int c, int target): c(c),target(target) { };
  int c;
  int target;
  void operator()(int c, int target) {
    this->c = c;
    this->target = target;
  }
};

struct Autom_State {
  Transition* tr;
  short cap;

  short outgoing;
  int incoming;
  bool isFinal;
  bool isDeleted;

  int transitionCount() const;
  void copyTransitions(const Autom_State& source, Autom_State* states);
  void trAdded();
  void trRemoved();
  int getTr(unsigned int c) const;
  void addTr(unsigned int c, int dest);
  void removeTr(unsigned int c);
  void reset();
  int getHash() const;
  bool operator==(const Autom_State& other) const;
};

class TransitionIterator {
public:
  TransitionIterator(const Autom_State& state):state(state) { begin(); }
  void begin();
  Transition next();
  bool hasNext();

private:
  const Autom_State& state;
  int currentIndex;
  int nextIndex;
};

void deallocateStates(Autom_State* ptr, int size);
Autom_State* allocateStates(int count);
Autom_State* reallocateStates(Autom_State* ptr, int oldSize, int newSize);

#endif
