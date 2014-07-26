#ifndef __AUTOM_STATE_H__
#define __AUTOM_STATE_H__

#define TR_SIZE 256

struct Transition {
  Transition(unsigned int c, int target): c(c),target(target) { };
  unsigned int c;
  int target;
};

struct Autom_State {
  Autom_State() { reset(); }

  bool isFinal;
  bool isDeleted;
  int tr[TR_SIZE];

  int outgoing;
  int incoming;

  int transitionCount() const;
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

#endif
