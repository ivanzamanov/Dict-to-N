#ifndef __AUTOM_H__
#define __AUTOM_H__

#include "stack.hpp"

#define TR_SIZE 256

struct Autom_State {
  Autom_State() {
    reset();
  }

  bool isFinal;
  bool isDeleted;
  int tr[TR_SIZE];

  void reset() {
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
  };

  ~Autom() {
    free(states);
  };

  void add(char* const w, int n);
  void remove(char* const w);
  int get(char* const w);

private:
  int cap;
  int last;
  Autom_State* states;

  Stack deleted;

  void expand();
  int newState();
  void delState(int s);
  int traverse(char* &w);
  int addTr(int src, unsigned int c, int dest);
  int getTr(int src, unsigned int c);

  int findEquiv(int state);
  void addEquiv(int state);
};

struct entry {
  entry(const int key, int hash):key(key),hash(hash), next(0) { };

  int key;
  int hash;
  entry* next;
};

#define HASH_INIT_SIZE 8
#define HASH_LOAD_FACTOR 0.5
struct hash {
  hash():size(0), cap(HASH_INIT_SIZE) {
    table = new entry*[cap];
  }

  int add(const int key, int hashCode);
  int get(const int key, int hashCode) const;
  int remove(const int key, int hashCode);

  //private:
  void expand();

  entry** table;
  int size;
  int cap;
};

#endif
