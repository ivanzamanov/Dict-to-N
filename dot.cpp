#include<stdio.h>

#include"dot.hpp"

void DotPrinter::start() {
  file = fopen(filePath, "w");
  fprintf(file, "digraph {\nrankdir = LR;\n");
}

void DotPrinter::end() {
  fprintf(file, "}\n");
  fclose(file);
}

void DotPrinter::edge(int src, char label, int dest) {
  fprintf(file, "%d -> %d [label=\"%c\"]\n", src, dest, label);
}
