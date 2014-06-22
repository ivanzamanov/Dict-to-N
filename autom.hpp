#ifndef __AUTOM_H__
#define __AUTOM_H__

#include "stack.hpp"

#define TR_SIZE 256

struct Autom_State {
  Autom_State() {
    reset();
  }
  bool isFinal;
  bool isDeleted;
  int tr[256];

  void reset() {
      isDeleted = 0;
      isFinal = 0;
      for(int i=0; i<TR_SIZE; i++)
	tr[i] = -1;
  }
};

class Autom {
public:
  Autom():cap(4), last(0), states(0) {
    states = (Autom_State*) realloc(states, cap * sizeof(Autom_State));
    for(int i=0; i < cap; i++) {
      states[i].reset();
    }
  };

  ~Autom() {
    free(states);
  };

  void add(char* const w, int n);
  void remove(char* const w);
  int get(char* const w);

private:
  int cap;
  int last;
  Autom_State* states;

  Stack deleted;

  void expand();
  int newState();
  void delState(int s);
  int traverse(char* &w);
  int addTr(int src, unsigned int c, int dest);
  int getTr(int src, unsigned int c);
};

#endif
