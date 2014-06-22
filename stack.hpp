#ifndef __STACK_H_
#define __STACK_H_

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

#endif /* STACK_H_ */
