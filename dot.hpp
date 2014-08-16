#ifndef __DOT_H__
#define __DOT_H_

#include<cstdio>

struct DotPrinter {
  DotPrinter(const char* const filePath): filePath(filePath) { };
  const char* const filePath;
  void start();
  void end();
  void edge(int src, char label, int  dest, int payload);
  void node(int id, bool isFinal);
private:
  FILE* file;
};

#endif
