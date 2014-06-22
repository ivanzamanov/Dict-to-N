#ifndef __AUTOM_H__
#define __AUTOM_H__

#include "word.h"
#include "stack.h"

class Autom {
 public:
  void add(const word& w, int n) const;

  void get(const word& w) const;

 private:
  int states;
  
}

#endif
