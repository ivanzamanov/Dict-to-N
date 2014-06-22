#ifndef STACK_H_
#define STACK_H_

class Stack {
public:
  Stack();
  Stack(const Stack& s);
  const Stack& operator=(const Stack& s);
  ~Stack();

  void push(int el);
  int pop();
  int peek() const;
  bool isEmpty() const;

private:
  int* data;
  int cap;
  int n;
};

Stack::Stack():cap(8), n(0) {
  data = (int*) malloc(cap * sizeof(int));
  n = -1;
}

Stack::Stack(const Stack& s):cap(s.cap), n(s.n) {
  data = (int*) malloc(cap * sizeof(int));
  for(int i=0; i<n; i++)
    data[i] = s.data[i];
}

Stack::~Stack() {
  free(data);
}

const Stack& Stack::operator =(const Stack& s) {
  free(data);
  cap = s.cap;
  n = s.n;
  data = (int*) malloc(cap * sizeof(int));
  for(int i=0; i<n; i++)
    data[i] = s.data[i];
  return *this;
}

void Stack::push(int el) {
  if(n == cap - 1) {
    cap = cap * 2;
    data = (int*) realloc(data, sizeof(int) * cap);
  }

  data[++n] = el;
}

int Stack::pop() {
  return data[n--];
}

int Stack::peek() const {
  return data[n];
}

bool Stack::isEmpty() const {
  return n < 0;
}

#endif /* STACK_H_ */
