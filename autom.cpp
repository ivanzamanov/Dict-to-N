#include<cstdlib>
#include<cstdio>

#include"autom.hpp"

void Autom::expand() {
  int newSize = cap * 2;
  states = (Autom_State*) realloc(states, newSize * sizeof(Autom_State));
  for(int i=cap; i < newSize; i++) {
    states[i].reset();
  }
  cap = newSize;
};

int Autom::newState() {
  int state;
  if(deleted.isEmpty()) {
    if(last == cap - 1)
      expand();
    state = ++last;
    return state;
  } else {
    state = deleted.pop();
  }
  states[state].reset();
  return state;
};

void Autom::delState(int s) {
  states[s].isDeleted = 1;
  deleted.push(s);
}

int Autom::addTr(int src, unsigned int c, int dest) {
  states[src].tr[c] = dest;
  return dest;
};

int Autom::getTr(int src, unsigned int c) {
  if(src < 0 || src > last || states[src].isDeleted)
    return -1;
  int dest = states[src].tr[c];
  if(states[dest].isDeleted)
    return -1;
  return dest;
};

int Autom::get(char* const w) {
  char* str = w;
  int state = 0;
  while(state != -1 && *str) {
    state = getTr(state, *str);
    str++;
  }
  printf("Get: %d\n", state);
  return state != -1 && states[state].isFinal;
}

int Autom::traverse(char* &w) {
  int state = 0;
  int next = 0;
  while(next != -1 && *w) {
    next = getTr(next, *w);
    if(next != -1) {
      state = next;
      w++;
    }
  }
  return state;
}

void Autom::add(char* const w, int n) {
  char* str = w;
  Stack cloned;
  int state = traverse(str);
  while(*str != 0) {
    int nState = newState();
    state = addTr(state, *str, nState);
    str++;
  }
  states[state].isFinal = 1;
  printf("Add: %d\n", state);
}

void remove(char* const w) {

}
