#ifndef __AUTOM_H__
#define __AUTOM_H__

#include "stack.hpp"
#include "autom_state.hpp"

class Autom;
struct entry;
struct hash;

struct hash {
  hash(const Autom& automaton);

  int add(const int key, int hashCode);
  int get(const int key, int hashCode) const;
  int remove(const int key, int hashCode);
  int getSlow(int key) const;

  void print();

  ~hash();
private:
  void checkSize(int atIndex);
  void expand();

  entry** table;
  int* sizes;
  int size;
  int cap;
  const Autom& automaton;
};

struct TrvEntry {
  TrvEntry(const TrvEntry& e):
    targetState(e.targetState),ch(e.ch),output(e.output) {};
  TrvEntry(int targetState, int ch, int output):
    targetState(targetState),ch(ch),output(output) {};
  int targetState;
  int ch;
  int output;
};
typedef Stack<TrvEntry> TrvStack;

class Autom {
public:
  Autom();
  ~Autom();

  void add(const char* const w, int n = 1);
  void remove(const char* const w);
  int get(const char* const w) const;

  bool equalStates(int s1, int s2) const;

  void printDot(const char* filePath = 0);
  void printWords();
  void printEquivs();
  void checkMinimal();
  void printStats();
  int getSize() const;
  int getStateCount() const;
  int getTransitionCount() const;

private:
  int cap;
  int last;
  Autom_State* states;
  IntStack deleted;
  hash* equivs;
  int size;
  AutomAllocator alloc;

  int clone(int src, Transition& tr);
  void expandCapacity();
  int newState();
  void delState(int s);
  void addTr(int src, const Transition& tr, bool isReplace);
  Transition getTr(int src, unsigned int c) const;
  void removeTr(int src, unsigned int c);

  void expandForAdd(TrvStack& cloned, const char* &str, int value);
  void expandForRemove(TrvStack& cloned, const char* &str);
  void reduceForAdd(TrvStack& cloned, const char* &str);
  void reduceForRemove(TrvStack& cloned, const char* &str);

  int findEquiv(int state);
  void addEquiv(int state);
  void removeEquiv(int state);
  void printHelper(int state, Stack<char>& stack);
};

struct entry {
  entry(const int key, int hash):key(key),hash(hash), next(0) { };

  int key;
  int hash;
  entry* next;
};

#endif
