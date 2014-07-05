#ifndef __HASH_H__
#define __HASH_H__

template<class T>
struct entry {
  entry(T* key, int hash):key(key),hash(hash) { };
  T* key;
  int hash;
  entry<T>* next = 0;
};

template<class T>
struct hash {
  entry<T>* table;
  int cap;
  int size;

  T* add(T* key, int hashCode);
  bool contains(T* key);
};

template<class T>
T* hash<T>::add(T* key, int hashCode) {
  int index = hashCode % cap;
  entry<T>* next = table[index];

  while(next != 0 && next->hash != hashCode && !*(next->key) == *key) {
    next = next->next;
  }

  if(next == 0) {
    next = new entry<T>(key, hashCode);
    next->next = table[index];
    table[index] = next;
  }

  return next->key;
}

template<class T>
T* hash<T>::contains(T* key) {
  
}

#endif
