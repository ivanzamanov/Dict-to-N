#include<cstdio>
#include<unordered_map>
#include<functional>

#include"autom.hpp"
#include"dot.hpp"

Autom::Autom():cap(4), last(0), size(0) {
  states = allocateStates(cap);
  states[0].incoming = 1;
  equivs = new hash(*this);
};

Autom::~Autom() {
  deallocateStates(states, cap);
  delete equivs;
};

void Autom::expandCapacity() {
  int newSize = cap * 2;
  states = reallocateStates(states, cap, newSize);
  for(int i=cap; i < newSize; i++) {
    states[i].reset();
  }
  cap = newSize;
};

static int stateCount = 0;
int Autom::newState() {
  int state;
  if(deleted.isEmpty()) {
    if(last == cap - 1)
      expandCapacity();
    state = ++last;
  stateCount++;
  if(stateCount % 100000 == 0)
    printf("States: %d\n", stateCount);
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
  TransitionIterator it(states[s]);
  while(it.hasNext()) {
    Transition tr = it.next();
    int dest = tr.target;
    states[dest].trRemoved();
    if(states[dest].incoming == 0)
      delState(dest);
  }
}

int Autom::addTr(int src, unsigned int c, int dest) {
  states[src].addTr(c, dest);
  states[dest].trAdded();
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
  states[src].removeTr(c);
  states[dest].trRemoved();
  if(states[dest].incoming == 0)
    delState(dest);
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
int Autom::clone(int src, unsigned int c) {
  int oldDest = getTr(src, c);
  Autom_State& oldDestState = states[oldDest];
  int result;
  if(oldDestState.incoming == 1)
    result = oldDest;
  else {
    result = newState();
    Autom_State& resultState = states[result];
    resultState.copyTransitions(oldDestState, states);
    resultState.isFinal = oldDestState.isFinal;
    resultState.outgoing = oldDestState.outgoing;
    removeTr(src, c);
    addTr(src, c, result);
  }
  return result;
}

TraverseResult Autom::expand(IntStack& cloned, const char* &str, bool forDelete) {
  int state = 0;
  int prev = 0;
  TraverseResult result;
  int i = 0;
  cloned.push(state);
  while(*str && getTr(state, *str) != -1) {
    prev = state;
    i++;

    removeEquiv(prev);
    state = clone(prev, *str);
    cloned.push(state);
    str++;
  }
  if(forDelete)
    return result;
  //  removeEquiv(state);
  //  cloned.push(state);
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
  int equiv;
  while(cloned.size() > 1
	&& (equiv = findEquiv(cloned.peek())) > 0) {
    state = cloned.peek();
    i++;
    // state = cloned.peek();
    // Look for an equivalent state
    // equiv = findEquiv(state);
    // if(equiv == -1 || equiv == state)
    //  break; // Not found -> no more will be found
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
  // Means we already recognize the word
  // if(!states[cloned.peek()].isFinal)
  //  size++;
  reduce(cloned, str);

#ifdef DEBUG
  printf("Add %s ", w);
  checkMinimal();
#endif
}

void Autom::remove(const char* const w) {
  // TODO: check if word is accepted before cloning
  const char* str = w;
  IntStack cloned;
  expand(cloned, str, 1);
  int prev = 0;
  while(cloned.size() > 1) {
    str--;
    cloned.pop();
    prev = cloned.peek();
    removeTr(prev, *str);
    if(states[prev].outgoing > 0 || states[prev].isFinal)
      break;
  }
  reduce(cloned, str);
#ifdef DEBUG
  printf("Removing %s ", w);
  checkMinimal();
#endif
}

void Autom::printDot(const char* filePath) {
  DotPrinter p(filePath);
  p.start();
  for(int i=0; i<=last; i++) {
    Autom_State& state = states[i];
    if(state.isDeleted)
      continue;
    p.node(i, state.isFinal);
    TransitionIterator it(state);
    while(it.hasNext()) {
      Transition tr = it.next();
      if(!states[tr.target].isDeleted)
	p.edge(i, tr.c, tr.target);
    }
  }
  p.end();
}

int Autom::getSize() const {
  return size;
}

void Autom::printWords() {
  Stack<char> stack;
  printHelper(0, stack);
}

void Autom::printHelper(int state, Stack<char>& stack) {
  const Autom_State& st = states[state];
  if(st.isFinal) {
    for(int i=0; i < stack.size(); i++)
      printf("%c", stack.getData()[i]);
    printf("\n");
  }
  TransitionIterator it(st);
  while(it.hasNext()) {
    Transition tr = it.next();
    stack.push(tr.c);
    printHelper(tr.target, stack);
    stack.pop();
  }
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

  int newCls = 2;
  bool changed = true;
  // TODO: currently, each class is traversed only once, which is not correct
  while(changed) {
    changed = false;
    for(int firstState = 0; firstState < last; firstState++) {
      Autom_State& first = states[firstState];
      if(first.isDeleted)
	continue;
      int firstStateClass = classes[firstState];
      TransitionIterator it(first);
      while(it.hasNext()) {
	Transition tr = it.next();
	int i = tr.c;
	int firstDest = tr.target;
	int firstDestClass = firstDest == -1 ? -1 : classes[firstDest];
	CheckHashMap m;
	m.emplace(Check(firstDestClass, i), firstStateClass);
	// for all other states, check if they have the same destination class
	// as the first
	for(int s = 0; s <= last; s++) {
	  Autom_State& state = states[s];
	  if(state.isDeleted || classes[s] != firstStateClass || s == firstState)
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
    }
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
