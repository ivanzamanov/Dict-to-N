#ifndef __AUTOM_H__
#define __AUTOM_H__

#define TR_SIZE 256
#include "stack.hpp"

class Autom;
struct Autom_State;
struct entry;
struct hash;
struct TraverseResult;

#define HASH_INIT_SIZE 7
#define HASH_LOAD_FACTOR 0.5
struct hash {
  hash(const Autom& automaton):size(0), cap(HASH_INIT_SIZE), automaton(automaton) {
    table = new entry*[cap];
    sizes = new int[cap];
    for(int i=0; i<cap; i++) {
      table[i] = 0;
      sizes[i] = 0;
    }
  }

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

struct Autom_State {
  Autom_State() { reset(); }

  bool isFinal;
  bool isDeleted;
  int tr[TR_SIZE];

  int outgoing;
  int incoming;

  inline void trAdded() {
    incoming++;
  }

  inline void trRemoved() {
    incoming--;
  }

  inline int getTr(unsigned int c) {
    return tr[c];
  }

  inline void addTr(unsigned int c, int dest) {
    if(dest == -1) // If this is a delete
      outgoing--;
    else if(tr[c] == -1) // If this is not a replace
      outgoing++;

    tr[c] = dest;
  }

  inline void removeTr(unsigned int c) {
    outgoing--;
    tr[c] = -1;
  }

  inline void reset() {
    incoming = 0;
    outgoing = 0;
    isDeleted = 0;
    isFinal = 0;
    for(int i=0; i<TR_SIZE; i++)
      tr[i] = -1;
  }

  int getHash() {
    int result = 0;
    for(int i=0; i<TR_SIZE; i++) {
      if(tr[i] >= 0) {
	result = (result + tr[i]) * TR_SIZE;
      }
    }
    return result;
  }

  bool operator==(const Autom_State& other) const {
    if(isFinal != other.isFinal)
      return 0;
    for(int i=0; i<TR_SIZE; i++) {
      if(tr[i] != other.tr[i])
	return 0;
    }
    return 1;
  }
};

class Autom {
public:
  Autom():cap(4), last(0) {
    states = (Autom_State*) malloc(cap * sizeof(Autom_State));
    states[0].reset();
    states[0].incoming = 1;
    equivs = new hash(*this);
  };

  ~Autom() {
    free(states);
    delete equivs;
  };

  void add(const char* const w, int n = 1);
  void remove(const char* const w);
  int get(const char* const w) const;

  bool equalStates(int s1, int s2) const;

  void printDot(const char* filePath = 0);
  void printWords();
  void checkMinimal();

private:
  int cap;
  int last;
  Autom_State* states;
  IntStack deleted;
  hash* equivs;

  inline int clone(int src, unsigned int c, bool cloneFinal);
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
