#include<stdio.h>

#include"dot.hpp"

void DotPrinter::start() {
  if(filePath != 0) {
    file = fopen(filePath, "w");
    fprintf(file, "digraph {\nrankdir = LR;\n");
  } else
    printf("digraph {\nrankdir = LR;\n");
}

void DotPrinter::end() {
  if(filePath != 0) {
    fprintf(file, "}\n");
    fclose(file);
  } else
    printf("}\n");
}

void DotPrinter::edge(int src, char label, int dest, int payload) {
  if(filePath != 0)
    fprintf(file, "%d -> %d [label=\"%c:%d\"]\n", src, dest, label, payload);
  else
    printf("%d -> %d [label=\"%c\"]\n", src, dest, label);
}

void DotPrinter::node(int id, bool isFinal) {
  if(filePath != 0)
    fprintf(file, "%d [shape=%s]\n", id, isFinal ? "doublecircle" : "circle");
  else
    printf("%d [shape=%s]\n", id, isFinal ? "doublecircle" : "circle");
}
