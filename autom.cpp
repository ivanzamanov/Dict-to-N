#include<cstdio>
#include<unordered_map>

#include"autom.hpp"
#include"dot.hpp"

Autom::Autom():cap(4), last(0), size(0) {
  states = alloc.allocateStates(cap);
  states[0].init(alloc);
  states[0].incoming = 1;
  equivs = new hash(*this);
};

Autom::~Autom() {
  alloc.deallocateStates(states, size);
  delete equivs;
};

int Autom::getStateCount() const {
  return last + 1 - deleted.size();
}

int Autom::getTransitionCount() const {
  int result = 0;
  for(int i=0; i<=last; i++) {
    if(!states[i].isDeleted()) {
      TransitionIterator it(states[i]);
      while(it.hasNext()) {
	it.next();
	result++;
      }
    }
  }
  return result;
}

void Autom::expandCapacity() {
  int newSize = cap * 1.6;
  states = alloc.reallocateStates(states, cap, newSize);
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
  states[state].init(alloc);
  return state;
};

bool Autom::equalStates(int s1, int s2) const {
  return states[s1] == states[s2];
}

void Autom::delState(int s) {
  states[s].destroy(alloc);
  deleted.push(s);
  TransitionIterator it(states[s]);
  while(it.hasNext()) {
    Transition tr = it.next();
    int dest = tr.target;
    states[dest].trRemoved();
    if(states[dest].incoming == 0)
      delState(dest);
  }
}

void Autom::addTr(int src, const Transition& tr, bool isReplace = false) {
  states[src].addTr(alloc, tr);
  if(!isReplace)
    states[tr.target].trAdded();
};

Transition Autom::getTr(int src, unsigned int c) const {
  if(src < 0 || src > last || states[src].isDeleted())
    return Transition(-1, -1);
  Transition trans = states[src].getTr(c);
  return trans;
};

void Autom::removeTr(int src, unsigned int c) {
  Transition trans = getTr(src, c);
  if(trans.target == -1)
    return;
  states[src].removeTr(Transition(c));
  states[trans.target].trRemoved();
  if(states[trans.target].incoming == 0)
    delState(trans.target);
}

int Autom::get(const char* const w) const {
  const char* str = w;
  int state = 0;
  int output = 0;
  while(state != -1 && *str) {
    Transition tr = getTr(state, *str);
    state = tr.target;
    output += tr.payload;
    str++;
  }
  return (state != -1 && states[state].isFinal) * output;
}

int Autom::findEquiv(int state) {
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
int Autom::clone(int src, Transition& tr) {
  int oldDest = tr.target;
  Autom_State& oldDestState = states[oldDest];
  int result;
  if(oldDestState.incoming == 1) {
    addTr(src, tr);
    oldDestState.incoming--;
    result = oldDest;
  } else {
    result = newState();
    Autom_State& resultState = states[result];
    alloc.copyTransitions(oldDestState, resultState, states);

    resultState.isFinal = oldDestState.isFinal;
    resultState.outgoing = oldDestState.outgoing;
    resultState.payload = oldDestState.payload;
    removeTr(src, tr.c);
    tr = Transition(tr.c, result, tr.payload);
    addTr(src, tr);
  }
  return result;
}

static int min(int i1, int i2) {
  return (i1 > i2) ? i2 : i1;
}

static int lcp(int i1, int i2) {
  return min(i1, i2);
}

static int sum(int i1, int i2) {
  return i1 + i2;
}

void Autom::expandForAdd(TrvStack& cloned, const char* &str, int n) {
  int state = 0;
  int prev = 0;
  Transition tr(0);
  int currentOutput = 0;
  int totalOutput = 0;
  cloned.push(TrvEntry(state, 0, 0));
  while(*str && (tr = getTr(state, *str)).target != -1) {
    int prevOutput = currentOutput;
    currentOutput = lcp(currentOutput + tr.payload, n);
    totalOutput = sum(totalOutput, tr.payload);

    prev = state;
    removeEquiv(prev);
    
    tr.payload = sum(-prevOutput, currentOutput);
    state = clone(prev, tr);
    TransitionIterator it(states[state]);
    while(it.hasNext()) {
      Transition& nextTr = it.next();
      if(nextTr.c != *(str+1))
	nextTr.payload = sum(-currentOutput, sum(totalOutput, nextTr.payload));
    }
    if(states[state].isFinal)
      states[state].payload = sum(-currentOutput, sum(totalOutput, states[state].payload));
    cloned.push(TrvEntry(state, *str, tr.payload));
    str++;
  }
  removeEquiv(state);

  int remaining = sum(-min(currentOutput, n), n);
  while(*str != 0) {
    // Add new states until minimal except
    // in the new word
    int nState = newState();
    Transition toAdd(*str, nState, remaining);
    addTr(state, toAdd);
    cloned.push(TrvEntry(nState, *str, remaining));
    remaining = 0;
    state = nState;
    str++;
  }
  // Last added state - final so we'll
  // recognize the word
  states[cloned.peek().targetState].isFinal = 1;
  states[cloned.peek().targetState].payload = remaining;
}

void Autom::expandForRemove(TrvStack& cloned, const char* &str) {
  int state = 0;
  int prev = 0;
  Transition tr(0);

  cloned.push(TrvEntry(state, 0, 0));
  while(*str && (tr = getTr(state, *str)).target != -1) {
    prev = state;
    removeEquiv(prev);
    
    state = clone(prev, tr);
    cloned.push(TrvEntry(state, *str, tr.payload));
    str++;
  }
  removeEquiv(state);
}

static int lcpOutgoingTransitions(TransitionIterator& it, int except, unsigned int originalPayload) {
  if(!it.hasNext())
    return 0;
  int result = originalPayload;
  while(it.hasNext()) {
    Transition& tr = it.next();
    if(tr.c != except)
      result = lcp(result, tr.payload);
  }
  return result;
}

static void pushBack(Autom_State* states, TrvStack& cloned, int& lcpValue) {
  TrvEntry entry = cloned.pop();
  int currentState = entry.targetState;

  TransitionIterator it(states[currentState]);
  lcpValue = lcpOutgoingTransitions(it, 0, states[currentState].payload);
  if(states[currentState].isFinal) {
    lcpValue = lcp(states[currentState].payload, lcpValue);
    unsigned int& pl = states[currentState].payload;
    pl = sum(-lcpValue, pl);
  }
  it.begin();
  while(it.hasNext()) {
    Transition& tr = it.next();
    tr.payload -= lcpValue;
  }
}

void Autom::reduceForRemove(TrvStack& cloned, const char* &str) {
  int pushedBack;

  TrvEntry entry = cloned.peek();
  // TODO: Move to method
  states[entry.targetState].isFinal = false;
  states[entry.targetState].payload = NON_FINAL_PAYLOAD;

  pushBack(states, cloned, pushedBack);
  Transition tr(entry.ch, entry.targetState, sum(pushedBack, entry.output));
  addTr(cloned.peek().targetState, tr, true);
  if(states[entry.targetState].outgoing == 0)
    removeTr(cloned.peek().targetState, entry.ch);

  while(!cloned.isEmpty()
	&& states[cloned.peek().targetState].outgoing == 0
	&& !states[cloned.peek().targetState].isFinal) {
    entry = cloned.peek();
    pushBack(states, cloned, pushedBack);
    removeTr(cloned.peek().targetState, entry.ch);
  }

  while(cloned.size() > 1) {
    entry = cloned.peek();
    pushBack(states, cloned, pushedBack);
    Transition tr(entry.ch, entry.targetState, sum(pushedBack, entry.output));
    addTr(cloned.peek().targetState, tr, true);

    int equiv;
    if((equiv = findEquiv(entry.targetState)) <= 0)
      break;
    // Found an equivalent, add a transition
    // from the previous state in the chain to the equiv.
    // and delete the obsoleted state
    delState(entry.targetState);
    tr = Transition(entry.ch, equiv, sum(pushedBack, entry.output));
    addTr(cloned.peek().targetState, tr);
  }
  // All remaining states,
  // need to be added to the final machine
  while(cloned.size() > 1) {
    entry = cloned.peek();
    pushBack(states, cloned, pushedBack);
    addEquiv(entry.targetState);
  }
}

void Autom::reduceForAdd(TrvStack& cloned, const char* &str) {
  // Offset from the end of the string
  int state;
  int equiv;

  while(!cloned.isEmpty()
	&& states[cloned.peek().targetState].outgoing == 0
	&& !states[cloned.peek().targetState].isFinal) {
    TrvEntry entry = cloned.pop();
    state = entry.targetState;
    removeTr(cloned.peek().targetState, entry.ch);
  }
  while(!cloned.isEmpty()
	&& (equiv = findEquiv(cloned.peek().targetState)) > 0) {
    TrvEntry entry = cloned.peek();
    state = entry.targetState;
    // Found an equivalent, add a transition
    // from the previous state in the chain to the equiv.
    // and delete the obsoleted state
    cloned.pop();
    delState(state);
    addTr(cloned.peek().targetState, Transition(entry.ch, equiv, entry.output));
  }
  // All remaining states,
  // need to be added to the final machine
  while(!cloned.isEmpty())
    addEquiv(cloned.pop().targetState);
}

void Autom::add(const char* const w, int n) {
  const char* str = w;
  TrvStack cloned;
  expandForAdd(cloned, str, n);
  reduceForAdd(cloned, str);

#ifdef DEBUG
  printf("Add %s ", w);
  checkMinimal();
#endif
}

void Autom::remove(const char* const w) {
  // TODO: check if word is accepted before cloning
  const char* str = w;
  TrvStack cloned;
  expandForRemove(cloned, str);
  reduceForRemove(cloned, str);
#ifdef DEBUG
  printf("Removing %s ", w);
  checkMinimal();
#endif
}

// -------------------------------------------------
// ----------- Printing methods and stats ----------
// -------------------------------------------------
void Autom::printDot(const char* filePath) {
  DotPrinter p(filePath);
  p.start();
  for(int i=0; i<=last; i++) {
    Autom_State& state = states[i];
    if(deleted.contains(i))
      continue;
    p.node(i, state.payload, state.isFinal);
    TransitionIterator it(state);
    while(it.hasNext()) {
      Transition tr = it.next();
      if(!states[tr.target].isDeleted())
	p.edge(i, tr.c, tr.target, tr.payload);
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
    classes[i] = this->states[i].isFinal && !this->states[i].isDeleted();
  }

  int newCls = 2;
  bool changed = true;
  // TODO: currently, each class is traversed only once, which is not correct
  while(changed) {
    changed = false;
    for(int firstState = 0; firstState < last; firstState++) {
      Autom_State& first = states[firstState];
      if(first.isDeleted())
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
	  if(state.isDeleted() || classes[s] != firstStateClass || s == firstState)
	    continue;
	  int dest = state.getTr(i).target;
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
      if(classes[i] == classes[j] && !states[i].isDeleted() && !states[j].isDeleted())
	isMin = false;

  printf("Minimal: %d\n", isMin);
  delete classes;
}

void Autom::printEquivs() {
  equivs->print();
}

void Autom::printStats() {
  int counts[200];
  int diffs[200];
  for(int i=0; i<200; i++) {
    counts[i] = 0;
    diffs[i] = 0;
  }
  for(int i=0; i<=last; i++) {
    Autom_State& st = states[i];
    if(!st.isDeleted()) {
      counts[st.outgoing]++;
      diffs[st.outgoing] += (st.cap - st.outgoing);
    }
  }
  for(int i=0; i<200; i++)
    if(counts[i] > 0) {
      printf("Count %d = %d\n", i, counts[i]);
      printf("Diff %d\n", diffs[i]);
    }
}
