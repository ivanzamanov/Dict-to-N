#include<stdio.h>
#include<cstdlib>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#include"autom.hpp"

void doWork(char* data, int size);

int main(int argc, const char** argv) {
  argc = 2;
  argv[1] = "dict";
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
  // printf("Size: %ld\n", size);

  char* data = new char[size + 1];
  read(fd, data, size);
  data[size + 1] = 0;
  doWork(data, size);
  delete data;
}

void doWork(char* data, int size) {
  Autom a;
  // a.add("011");
  // a.add("010");
  // a.remove("011");
  a.add("011");
  a.checkMinimal();
  // a.add("0111");
  // a.add("11111");
  // a.add("0101");
  // a.remove("0111");
  // a.add("01110");

  // a.add("10");
  // a.remove("01110");
  // a.add("011");
  // a.add("1100");
  // a.add("11");
  
  // a.remove("0101");
  // a.remove("11111");

  a.printDot("dot.dot");
  a.printWords();
}
