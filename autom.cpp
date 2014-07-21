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

int Autom::addTr(int src, unsigned int c, int dest, bool equiv) {
  if(equiv)
    removeEquiv(src);
  states[src].addTr(c, dest);
  states[dest].trAdded();
  if(equiv)
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

void Autom::removeTr(int src, unsigned int c, bool equiv) {
  int dest = getTr(src, c);
  if(dest == -1)
    return;
  if(equiv)
    removeEquiv(src);
  states[src].removeTr(c);
  states[dest].trRemoved();
  if(states[dest].incoming == 0)
    delState(dest);
  if(equiv)
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
  /*  StateSet::const_iterator found = eqSet.find(state);
  if(found == eqSet.end())
    return -1;
  else
  return *found;*/
  int h = states[state].getHash();
  int result = equivs->get(state, h);
  return result;
}

void Autom::addEquiv(int state) {
  int h = states[state].getHash();
  equivs->add(state, h);
}

void Autom::removeEquiv(int state) {
  int h = states[state].getHash();
  equivs->remove(state, h);
}

// clone the destination for the given transition
int Autom::clone(int src, unsigned int c, bool equiv) {
  int oldDest = getTr(src, c);
  Autom_State& oldDestState = states[oldDest];
  int result;
  if(oldDestState.incoming == 1)
    result = oldDest;
  else {
    if(equiv)
      removeEquiv(src);
    result = newState();
    Autom_State& resultState = states[result];
    for(int i = 0; i<TR_SIZE; i++) {
      int copyDest = oldDestState.tr[i];
      resultState.tr[i] = copyDest;
      if(copyDest >= 0)
	states[copyDest].trAdded();
    }
    resultState.isFinal = oldDestState.isFinal;
    resultState.outgoing = oldDestState.outgoing;
    removeTr(src, c, false);
    addTr(src, c, result, false);
    // add again since this is still a part of the minimal
    if(equiv)
      addEquiv(src);
  }
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

      removeEquiv(prev);
      state = clone(prev, *str, false);
      cloned.push(state);
      str++;
    } else {
      break;
    }
  }
  if(forDelete)
    return result;
  removeEquiv(state);
  cloned.push(state);
  while(*str != 0) {
    // Add new states until minimal except
    // in the new word
    int nState = newState();
    addTr(state, *str, nState, false);
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
    addTr(cloned.peek(), *(str - i), equiv, false);
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
  printf("Add %s ", w);
  checkMinimal();
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
    removeTr(prev, *str, false);
    if(states[prev].outgoing > 0 || states[prev].isFinal)
      break;
  }
  reduce(cloned, str);
  printf("Removing %s ", w);
  checkMinimal();
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
    classes[i] = this->states[i].isFinal && !this->states[i].isDeleted;
  }

  int cls = 0;
  int newCls = 2;
  bool changed = true;
  // TODO: currently, each class is traversed only once, which is not correct
  while(changed) {
    changed = false;
    int firstState = 0;
    // find the first state of this class
    while(classes[firstState] != cls || states[firstState].isDeleted) {
      if(firstState == last) {
	break;
      }
      firstState++;
    }

    Autom_State& first = states[firstState];
    for(unsigned int i=0; i < TR_SIZE; i++) {
      int firstDest = first.getTr(i);
      int firstDestClass = firstDest == -1 ? -1 : classes[firstDest];
      CheckHashMap m;
      m.emplace(Check(firstDestClass, i), cls);
      // for all other states, check if they have the same destination class
      // as the first
      for(int s = firstState+1; s <= last; s++) {
	Autom_State& state = states[s];
	if(state.isDeleted || classes[s] != cls)
	  continue;
	int dest = state.getTr(i);
	int destCls = dest == -1 ? -1 : classes[dest];
	std::pair<CheckHashMap::iterator,bool> p = m.emplace(Check(destCls, i), newCls);
	if(p.second) {
	  classes[s] = newCls++;
	  changed = true;
	} else {
	  // partition to new class
	  CheckHashMap::iterator it = p.first;
	  int oldClass = classes[s];
	  classes[s] = p.first->second;
	  changed = changed || oldClass != classes[s];
	}
      }
    }
    cls++;
  }

  bool isMin = true;
  for(int i=0; i<last+1 && isMin; i++)
    for(int j=i+1; j<last+1 && isMin; j++)
      if(classes[i] == classes[j] && !states[i].isDeleted && !states[j].isDeleted)
	isMin = false;

  printf("Minimal: %d\n", isMin);
  delete classes;
}

void Autom::printEquivs() {
  equivs->print();
}
