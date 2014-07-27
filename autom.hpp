#ifndef __AUTOM_H__
#define __AUTOM_H__

#include "stack.hpp"
#include "autom_state.hpp"

class Autom;
struct entry;
struct hash;
struct TraverseResult;

struct hash {
  hash(const Autom& automaton);

  int add(const int key, int hashCode);
  int get(const int key, int hashCode) const;
  int remove(const int key, int hashCode);

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
  int getSize() const;

private:
  int cap;
  int last;
  Autom_State* states;
  IntStack deleted;
  hash* equivs;
  int size;

  inline int clone(int src, unsigned int c);
  inline void expandCapacity();
  inline int newState();
  inline void delState(int s);
  inline int addTr(int src, unsigned int c, int dest);
  inline int getTr(int src, unsigned int c) const;
  inline void removeTr(int src, unsigned int c);

  inline TraverseResult expand(IntStack& cloned, const char* &str, bool forDelete = 0);
  inline void reduce(IntStack& cloned, const char* &str);

  inline int findEquiv(int state);
  inline void addEquiv(int state);
  inline void removeEquiv(int state);
  void printHelper(int state, Stack<char>& stack);
};

struct entry {
  entry(const int key, int hash):key(key),hash(hash), next(0) { };

  int key;
  int hash;
  entry* next;
};

struct TraverseResult {
  TraverseResult():lastBranch(-1) { };
  int lastBranch;
};

#endif
