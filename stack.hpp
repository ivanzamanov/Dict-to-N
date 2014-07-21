#ifndef __STACK__H__
#define __STACK__H__

#include<cstdio>
#include<cstdlib>

template<class T>
class Stack {
public:
  Stack();
  Stack(const Stack<T>& s);
  const Stack& operator=(const Stack<T>& s);
  ~Stack();

  void push(T el);
  T pop();
  T peek() const;
  bool isEmpty() const;
  int size() const;

  void print() const;

private:
  T* data;
  int cap;
  int n;
};

typedef Stack<int> IntStack;

template<class T>
Stack<T>::Stack():cap(8), n(0) {
  data = (T*) malloc(cap * sizeof(T));
  n = -1;
}

template<class T>
Stack<T>::Stack(const Stack<T>& s):cap(s.cap), n(s.n) {
  data = (T*) malloc(cap * sizeof(int));
  for(int i=0; i<n; i++)
    data[i] = s.data[i];
}

template<class T>
Stack<T>::~Stack() {
  free(data);
}

template<class T>
const Stack<T>& Stack<T>::operator =(const Stack<T>& s) {
  free(data);
  cap = s.cap;
  n = s.n;
  data = (T*) malloc(cap * sizeof(T));
  for(int i=0; i<n; i++)
    data[i] = s.data[i];
  return *this;
}

template<class T>
void Stack<T>::push(T el) {
  if(n == cap - 1) {
    cap = cap * 2;
    data = (T*) realloc(data, sizeof(T) * cap);
  }

  data[++n] = el;
}

template<class T>
T Stack<T>::pop() {
  return data[n--];
}

template<class T>
T Stack<T>::peek() const {
  return data[n];
}

template<class T>
bool Stack<T>::isEmpty() const {
  return n < 0;
}

template<class T>
int Stack<T>::size() const {
  return n+1;
}

template<class T>
void Stack<T>::print() const {
  for(int i=n; i >= 0; i--)
    printf("%d ", data[i]);
  printf("\n");
}

#endif
