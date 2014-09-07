#include"autom.hpp"

#define HASH_LOAD_FACTOR 0.5

static entry* findInTable(int key, int hashCode, entry** table, int cap, const Autom& automaton) {
  int index = hashCode % cap;
  entry *next = table[index];

  while(next != 0) {
    if(next->hash == hashCode &&
       (next->key == key
	|| automaton.equalStates(next->key, key))) {
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

hash::hash(const Autom& automaton):size(0), cap(7), automaton(automaton) {
  table = new entry*[cap];
  sizes = new int[cap];
  for(int i=0; i<cap; i++) {
    table[i] = 0;
    sizes[i] = 0;
  }
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
  delete[] table;
  delete[] sizes;
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
  delete[] table;
  table = new_table;
  delete[] sizes;
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

int hash::getSlow(int key) const {
  for(int i=0; i<cap; i++) {
    entry* e = table[i];
    while(e != 0) {
      if(e->key == key)
	return e->key;
      e = e->next;
    }
  }
  return -1;
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
  if(next != 0 &&
     next->hash == hashCode &&
     next->key == key &&
     automaton.equalStates(next->key, key)) {
    table[index] = next->next;
    delete next;
    return key;
  }

  entry *prev = next;
  next = next->next;
  while(next != 0 &&
	(
	 next->hash != hashCode ||
	 next->key != key ||
	 !automaton.equalStates(next->key, key)
	 )
	) {
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

void hash::print() {
  for(int i=0; i<cap; i++) {
    entry* e = table[i];
    if(e == 0)
      continue;
    while(e != 0) {
      printf("{k = %d, h = %d} ", e->key, e->hash);
      e = e->next;
    }
    printf("\n");
  }
}

