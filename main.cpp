#include<stdio.h>
#include<cstdlib>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#include"word.h"
#include"stack.hpp"
#include"autom.hpp"

void doWork(char* data, int size);

int main(int argc, const char** argv) {
  if(argc < 2) {
    printf("No input file specified\n");
    return 1;
  }

  int fd = open(argv[1], O_RDONLY);
  if(fd < 0) {
    printf("Cannot open file %s\n", argv[1]);
    return 1;
  }

  struct stat fileStat;
  if(fstat(fd, &fileStat) < 0) {
    printf("Cannot stat file %s\n", argv[1]);
    return 1;
  }
  long int size = fileStat.st_size;
  printf("Size: %ld\n", size);

  char* data = new char[size + 1];
  read(fd, data, size);
  data[size + 1] = 0;
  doWork(data, size);
  delete data;
}

void printHash(hash& h) {
  for(int i=0; i<h.cap; i++) {
    printf("%d: ", i);
    entry* e = h.table[i];
    while(e != 0) {
      printf("{k = %d, h = %d} ", e->key, e->hash);
      e = e->next;
    }
    printf("\n");
  }
}

void doWork(char* data, int size) {
  hash h;
  const int count = 10000;
  for(int i=0; i<count; i++) {
    h.add(i, i);
  }
  printHash(h);
  for(int i=0; i<count; i++) {
    printf("%d\n", h.get(i, i));
  }
  printf("Size: %d\n", h.size);
  for(int i=0; i<count; i++) {
    printf("%d\n", h.remove(i, i));
  }
  printf("Cap: %d\n", h.cap);

  // Autom a;
  // char* w = "test";
  // a.add(data, 0);
  // printf("%d\n", a.get(data));
}
