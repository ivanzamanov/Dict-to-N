#include<cstdlib>
#include<cstdio>

#include"autom.hpp"

struct entry {
  entry(int key, int hash):key(key),hash(hash) { };
  int key;
  int hash;
  entry* next = 0;
};

struct hash {
  entry* table;
  int cap;
  int size;

  void add(Autom_State& state) {
    int h = state.getHash();
    int index = hash % cap;
    entry* e;
    if(table[cap] == 0) {
      e = new entry(key, h);
      table[cap] = e;
    } else {
      
    }
  }
};

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
  if(dest == -1 || states[dest].isDeleted)
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
    cloned.push(state);
    str++;
  }
  int i=0;
  while(!cloned.isEmpty()) {
    state = cloned.pop();
    int equiv = findEquiv(state);
    if(equiv == -1)
      break;
    int prev = cloned.peek();
    addTr(prev, *(str - i), equiv);
    delState(state);
  }

  while(!cloned.isEmpty()) {
    addEquiv(equivs.add(cloned.pop();
  }
  states[state].isFinal = 1;
}

void remove(char* const w) {

}
