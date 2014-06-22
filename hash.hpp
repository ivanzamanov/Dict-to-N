#include<

const int P = 53;
const int C = 50;
const double LOAD_FACTOR = 0.8d;
const int MAX_CAP = 32;

typedef int key_t;

template<class T>
class hash {
public:
  hash();
  hash(const hash& other);

  int cap = 4;
  int size = 0;

  void insert();
  const T& get(const key_t& key, const T& def);

private:
  void expand();
  int get_hash(const key_t key, const int& i);

  int limit = 4;

  struct entry {
    key_t key;
    T data;
  };

  entry* table;
};

template<class T>
hash<T>::hash() {
  hash<T>& h = *this;
  h.cap = 4;
  h.size = 0;
  h.table = new entry[cap];
  for (int i=0; i<cap; i++) {
    h.table[i].key = -1;
  }
}

template<class T>
hash<T>::hash(const hash& other) {
  hash<T>& h = *this;
  h.cap = other.cap;
  h.size = other.size;
  h.table = new entry[cap];
  for (int i=0; i<cap; i++) {
    h.table[i].key = other.table[i].key;
    h.table[i].data = other.table[i].data;
  }
}

template<class T>
void hash<T>::insert(const key_t& key, const T& data) {
  size++;
  if(size >= cap * LOAD_FACTOR) {
    expand();
  }
  int i = 0;
  while(i <= limit) {
  int h = get_hash(key, i);
    entry& current = table[h];
    if(current.key == -1 || current.key == key) {
      current.key = key;
      current.data = data;
      return;
    }
    i++;
  }
  expand();
  size--;
  insert(key, data);
}

template<class T>
const T& hash<T>::get(const key_t& key, const T& def) {
  int i = 0;
  while(i <= limit) {
  int h = get_hash(key, i);
    entry& current = table[h];
    if(current.key == key) {
      return current.data;
    } else if(current.key == -1) {
      return def;
    }
    i++;
  }
  return def;
}

template<class T>
int hash<T>::get_hash(const key_t key, const int& i) {
  return ((C * key) % P + i) % this->cap;
}

template<class T>
void hash<T>::expand() {
  hash<T>& table = *this;
  if(table.cap == MAX_CAP) {
    return;
  }
  entry* old_table = table.table;
  int old_cap = table.cap;
  table.cap = old_cap * 2;
  if(table.cap > MAX_CAP) {
    table.cap = MAX_CAP;
  }
  table.table = new entry[table.cap];
  for(int i = 0; i < table.cap; i++) {
    table.table[i].key = -1;
  }
  table.size = 0;
  for (int i=0; i < old_cap; i++) {
    entry& entry = old_table[i];
    if(entry.key >= 0) {
      table.insert(entry.key, entry.data);
    }
  }
  limit = cap;
}
