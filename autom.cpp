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
    state = cloned.peek();
    int equiv = 0;//findEquiv(state);
    if(equiv == -1)
      break;
    cloned.pop();
    int prev = cloned.peek();
    addTr(prev, *(str - i), equiv);
    delState(state);
  }

  while(!cloned.isEmpty()) {
    //    addEquiv(equivs.add(cloned.pop()));
  }
  states[state].isFinal = 1;
}

void remove(char* const w) {

}

// ----- hash ----

static entry* findInTable(int key, int hashCode, entry** table, int cap, int& traversed) {
  traversed = 0;
  int index = hashCode % cap;
  entry *next = table[index];

  while(next != 0 && next->hash != hashCode && next->key != key) {
    next = next->next;
    traversed++;
  }
  return next;
}

static void addToTable(entry* e, entry** table, int cap) {
  int index = e->hash % cap;
  e->next = table[index];
  table[index] = e;
}

void hash::expand() {
  int new_cap = (cap + 1) * 2 - 1;
  entry** new_table = new entry*[new_cap];
  for(int i=0; i<new_cap; i++)
    new_table[i] = 0;
  for(int i = 0; i < cap; i++) {
    entry* e = table[i];
    entry* next;
    while(e != 0) {
      next = e->next;
      addToTable(e, new_table, new_cap);
      e = next;
    }
  }
  delete table;
  table = new_table;
  cap = new_cap;
}

int hash::add(int key, int hashCode) {
  int traversed;
  entry *e = findInTable(key, hashCode, table, cap, traversed);

  if(e == 0) {
    e = new entry(key, hashCode);
    addToTable(e, table, cap);
    size++;
  } // Else, return the equivalent found

  if(traversed > HASH_LOAD_FACTOR * cap) {
    expand();
  }
  return e->key;
}

int hash::get(int key, int hashCode) const {
  int traversed;
  entry *e = findInTable(key, hashCode, table, cap, traversed);

  if(e != 0)
    return e->key;
  else
    return -1;
}

int hash::remove(const int key, int hashCode) {
  int index = hashCode % cap;
  entry *next = table[index];
  if(next != 0 && next->hash == hashCode && next->key == key) {
    table[index] = next->next;
    delete next;
    return key;
  }

  entry *prev = next;
  next = next->next;
  while(next != 0 && next->hash != hashCode && next->key != key) {
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
