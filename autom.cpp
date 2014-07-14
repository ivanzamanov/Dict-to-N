#include<cstdlib>
#include<cstdio>
#include<unordered_map>
#include<functional>

#include"autom.hpp"
#include"dot.hpp"

void Autom::expandCapacity() {
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
      expandCapacity();
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
  if(states[s].outgoing == 0)
    return;
  for(unsigned int i = 0; i<TR_SIZE; i++) {
    int dest = states[s].getTr(i);
    if(dest >= 0) {
      states[dest].trRemoved();
      if(states[dest].incoming == 0)
	delState(dest);
    }
  }
}

int Autom::addTr(int src, unsigned int c, int dest) {
  bool isEquiv = findEquiv(src);
  if(isEquiv)
    removeEquiv(src);
  states[src].addTr(c, dest);
  states[dest].trAdded();
  if(isEquiv)
    addEquiv(src);
  return dest;
};

int Autom::getTr(int src, unsigned int c) const {
  if(src < 0 || src > last || states[src].isDeleted)
    return -1;
  int dest = states[src].getTr(c);
  if(dest == -1 || states[dest].isDeleted)
    return -1;
  return dest;
};

void Autom::removeTr(int src, unsigned int c) {
  int dest = getTr(src, c);
  if(dest == -1)
    return;
  bool isEquiv = findEquiv(src);
  if(isEquiv)
    removeEquiv(src);
  states[src].removeTr(c);
  states[dest].trRemoved();
  if(states[dest].incoming == 0)
    delState(dest);
  if(isEquiv)
    addEquiv(src);
}

int Autom::get(const char* const w) const {
  const char* str = w;
  int state = 0;
  while(state != -1 && *str) {
    state = getTr(state, *str);
    str++;
  }
  return state != -1 && states[state].isFinal;
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

// clone the destination for the given transition
int Autom::clone(int src, unsigned int c, bool cloneFinal) {
  removeEquiv(src);
  int oldDest = getTr(src, c);
  Autom_State& oldDestState = states[oldDest];
  if(oldDestState.incoming == 1 /*&& !(oldDestState.isFinal && !cloneFinal)*/)
    return oldDest;
  int result = newState();
  Autom_State& resultState = states[result];
  for(int i = 0; i<TR_SIZE; i++) {
    int copyDest = oldDestState.tr[i];
    resultState.tr[i] = copyDest;
    if(copyDest >= 0)
      states[copyDest].trAdded();
  }
  resultState.isFinal = cloneFinal;
  resultState.outgoing = oldDestState.outgoing;
  removeTr(src, c);
  addTr(src, c, result);
  // add again since this is still a part of the minimal
  addEquiv(src);
  return result;
}

TraverseResult Autom::expand(IntStack& cloned, const char* &str, bool forDelete) {
  int state;
  int prev = 0;
  TraverseResult result;
  int i = 0;
  while(*str) {
    prev = state;
    if(getTr(prev, *str) != -1) {
      i++;
      if(states[prev].outgoing > 1)
	result.lastBranch = i;
      state = clone(prev, *str, *(str + 1) == 0);
      if(forDelete)
	cloned.push(state);
      str++;
    } else {
      break;
    }
  }
  if(forDelete)
    return result;
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
  return result;
}

void Autom::reduce(IntStack& cloned, const char* &str) {
  // Offset from the end of the string
  int i = 0;
  int state;
  // Traversing the newly added states backwards
  while(cloned.size() > 1) {
    i++;
    state = cloned.peek();
    // Look for an equivalent state
    int equiv = findEquiv(state);
    if(equiv == -1 || equiv == state)
      break; // Not found -> no more will be found
    // Found an equivalent, add a transition
    // from the previous state in the chain to the equiv.
    // and delete the obsoleted state
    cloned.pop();
    delState(state);
    addTr(cloned.peek(), *(str - i), equiv);
  }
  // All remaining states, including the bottom,
  // need to be added to the final machine
  while(!cloned.isEmpty())
    addEquiv(cloned.pop());
}

void Autom::add(const char* const w, int n) {
  const char* str = w;
  IntStack cloned;
  expand(cloned, str);
  reduce(cloned, str);
}

void Autom::remove(const char* const w) {
  // TODO: check if word is accepted before cloning
  const char* str = w;
  IntStack cloned;
  expand(cloned, str, 1);
  while(cloned.size() > 1) {
    str--;
    cloned.pop();
    int prev = cloned.peek();
    removeTr(prev, *str);
    if(states[prev].outgoing > 0 || states[prev].isFinal)
      break;
  }
  reduce(cloned, str);
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
      int dest = state.getTr(j);
      if(dest >= 0 && !states[dest].isDeleted)
	p.edge(i, j, state.getTr(j));
    }
  }
  p.end();
}

void Autom::printWords() {
  
}

struct Check {
  Check(int destClass, unsigned int c)
    :destClass(destClass), c(c) { };
  int destClass;
  unsigned int c;
  bool operator==(Check& other) {
    return other.destClass == destClass && other.c == c;
  };
};

struct CheckHash
{
  std::size_t operator()(const Check &c) const
  {
    std::size_t x = c.destClass * c.c;
    return x;
  }
};

struct CheckEq
{
  bool operator()(const Check &c1, const Check &c2) const
  {
    return c1.c == c2.c && c1.destClass == c2.destClass;
  }
};

typedef std::unordered_map<Check, int, CheckHash, CheckEq> CheckHashMap;
void Autom::checkMinimal() {
  int* classes = new int[last + 1];
  for(int i=0; i<=last; i++) {
    classes[i] = this->states[i].isFinal;
  }
  //  states = (int*)realloc(states, sizeof(int) * j);
  int cls = 0;
  int newCls = 2;
  while(cls < newCls) {
    for(unsigned int i=0; i < TR_SIZE; i++) {
      CheckHashMap m;
      int firstState = 0;
      // find the first state of this class
      while(classes[firstState] != cls) {
	firstState++;
      }
	  
      Autom_State& first = states[0];
      int firstDest = first.getTr(i);
      int firstDestClass = firstDest == -1 ? -1 : classes[firstDest];
      // for all other states, check if they have the same destination class
      // as the first
      for(int s = 1; s <= last; s++) {
	Autom_State& state = states[s];
	if(state.isDeleted)
	  continue;
	int dest = state.getTr(i);
	int destCls;
	if(dest >= 0)
	  destCls = classes[dest];
	else
	  destCls = ++newCls;
	std::pair<CheckHashMap::iterator,bool> p = m.emplace(Check(destCls, i), newCls);
	if(p.second) {
	  classes[s] = newCls;
	  newCls++;
	} else {
	  // partition to new class
	  CheckHashMap::iterator it = p.first;
	  const Check check = (*it).first;
	  classes[s] = check.destClass;
	}
      }
    }
    cls++;
  }

  printf("Minimal: %d\n", newCls - 1 == last+1);
  delete states;
}

// -----
// ----- hash ----
// -----

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
  if(next != 0 && next->hash == hashCode && next->key == key && automaton.equalStates(next->key, key)) {
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
