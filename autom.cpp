#include<cstdlib>
#include<cstdio>

#include"autom.hpp"
#include"dot.hpp"

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

bool Autom::equalStates(int s1, int s2) const {
  return states[s1] == states[s2];
}

void Autom::delState(int s) {
  states[s].isDeleted = 1;
  deleted.push(s);
}

int Autom::addTr(int src, unsigned int c, int dest) {
  states[src].tr[c] = dest;
  return dest;
};

int Autom::getTr(int src, unsigned int c) const {
  if(src < 0 || src > last || states[src].isDeleted)
    return -1;
  int dest = states[src].tr[c];
  if(dest == -1 || states[dest].isDeleted)
    return -1;
  return dest;
};

int Autom::get(const char* const w) const {
  const char* str = w;
  int state = 0;
  while(state != -1 && *str) {
    state = getTr(state, *str);
    str++;
  }
  return state != -1 && states[state].isFinal;
}

int Autom::traverse(const char* &w) {
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

int Autom::findEquiv(int state) {
  return equivs->get(state, states[state].getHash());
}

void Autom::addEquiv(int state) {
  equivs->add(state, states[state].getHash());
}

void Autom::removeEquiv(int state) {
  equivs->remove(state, states[state].getHash());
}

void Autom::add(const char* const w, int n) {
  const char* str = w;
  IntStack cloned;
  int state = traverse(str);
  // Bottom of the stack - the last state
  // from the starting machine
  removeEquiv(state);
  cloned.push(state);
  while(*str != 0) {
    // Add new states until minimal except
    // in the new word
    int nState = newState();
    addTr(state, *str, nState);
    cloned.push(nState);
    state = nState;
    str++;
  }
  // Last added state - final so we'll
  // recognize the word
  states[cloned.peek()].isFinal = 1;

  // Offset from the end of the string
  int i=0;
  // Traversing the newly added states backwards
  while(cloned.size() > 1) {
    i++;
    // Look for an equivalent state
    int equiv = findEquiv(cloned.peek());
    if(equiv == -1)
      break; // Not found -> no more will be found
    // Found an equivalent, add a transition
    // from the previous state in the chain to the equiv.
    // and delete the obsoleted state
    state = cloned.pop();
    delState(state);
    addTr(cloned.peek(), *(str - i), equiv);
  }
  // All remaining states, including the bottom,
  // need to be added to the final machine
  while(!cloned.isEmpty())
    addEquiv(cloned.pop());
}

void Autom::remove(const char* const w) {

}

void Autom::printDot(const char* filePath) {
  DotPrinter p(filePath);
  p.start();
  for(int i=0; i<=last; i++) {
    Autom_State& state = states[i];
    if(state.isDeleted)
      continue;
    p.node(i, state.isFinal);
    for(unsigned int j=0; j<TR_SIZE; j++) {
      if(state.tr[j] >= 0)
	p.edge(i, j, state.tr[j]);
    }
  }
  p.end();
}

void Autom::printWords() {
  
}

// ----- hash ----

static entry* findInTable(int key, int hashCode, entry** table, int cap, const Autom& automaton) {
  int index = hashCode % cap;
  entry *next = table[index];

  while(next != 0) {
    if(next->hash == hashCode && (next->key == key || automaton.equalStates(next->key, key))) {
	return next;
      }
      next = next->next;
  }

  return next;
}

static void addToTable(entry* e, entry** table, int index, int* sizes) {
  e->next = table[index];
  sizes[index]++;
  table[index] = e;
}

void hash::expand() {
  int new_cap = (cap + 1) * 2 - 1;
  entry** new_table = new entry*[new_cap];
  int* new_sizes = new int[new_cap];
  for(int i=0; i<new_cap; i++) {
    new_table[i] = 0;
    new_sizes[i] = 0;
  }
  for(int i = 0; i < cap; i++) {
    entry* e = table[i];
    entry* next;
    while(e != 0) {
      next = e->next;
      int indexInTable = e->hash % new_cap;
      addToTable(e, new_table, indexInTable, new_sizes);
      e = next;
    }
  }
  delete table;
  table = new_table;
  delete sizes;
  sizes = new_sizes;
  cap = new_cap;
}

void hash::checkSize(int atIndex) {
  size++;
  if(sizes[atIndex] > HASH_LOAD_FACTOR * cap)
    expand();
}

int hash::add(int key, int hashCode) {
  entry *e = findInTable(key, hashCode, table, cap, automaton);

  if(e == 0) {
    e = new entry(key, hashCode);
    int indexInTable = e->hash % cap;
    addToTable(e, table, indexInTable, sizes);
    checkSize(indexInTable);
  } // Else, return the equivalent found

  return e->key;
}

int hash::get(int key, int hashCode) const {
  entry *e = findInTable(key, hashCode, table, cap, automaton);

  if(e != 0)
    return e->key;
  else
    return -1;
}

int hash::remove(const int key, int hashCode) {
  int index = hashCode % cap;
  entry *next = table[index];
  if(next == 0)
    return -1;
  if(next != 0 && next->hash == hashCode && next->key == key && !automaton.equalStates(next->key, key)) {
    table[index] = next->next;
    delete next;
    return key;
  }

  entry *prev = next;
  next = next->next;
  while(next != 0 && next->hash != hashCode && next->key != key && !automaton.equalStates(next->key, key)) {
    prev = next;
    next = next->next;
  }
  if(next != 0) {
    prev->next = next->next;
    delete next;
    return key;
  }

  return -1;
}

hash::~hash() {
  for(int i = 0; i < cap; i++) {
    entry* e = table[i];
    entry* next;
    while(e != 0) {
      next = e->next;
      delete e;
      e = next;
    }
  }
  delete table;
}

void hash::print() {
  for(int i=0; i<cap; i++) {
    printf("%d: ", i);
    entry* e = table[i];
    while(e != 0) {
      printf("{k = %d, h = %d} ", e->key, e->hash);
      e = e->next;
    }
    printf("\n");
  }
}
