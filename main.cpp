#include<stdio.h>
#include<cstdlib>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#include<vector>
#include<string>
#include<cstring>

#include"autom.hpp"

void doWork(char* data, int size);

int main(int argc, const char** argv) {
  argc = 2;
  argv[1] = "million.txt";
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

  char* data = new char[size + 1];
  read(fd, data, size);
  data[size] = 0;
  doWork(data, size);
}

typedef std::vector<char*> StringVector;
inline bool isWhitespace(char c) {
  return c && c == '\n';
}
void readStrings(char* data, int size, StringVector& vec) {
  int offset=0;
  while(offset < size) {
    int i=0;
    char* ptr = data + offset;
    while(!isWhitespace(*ptr) && i + offset < size) {
      i++;
      ptr++;
    }
    char* str = new char[i];
    strncpy(str, data + offset, i);
    str[i-1] = 0;
    offset += i;
    vec.push_back(str);
    offset++;
  }
}
void printStrings(StringVector& vec) {
  StringVector::iterator it = vec.begin();
  while(it != vec.end()) {
    printf("%s\n", *it);
    ++it;
  }
}
void doWork(char* data, int size) {
  Autom a;
  StringVector strings;
  readStrings(data, size, strings);
  StringVector::iterator it = strings.begin();
  int count = 0;
  while(it != strings.end()) {
    count++;
    a.add(*it);
    it++;
    if(count % 1000 == 0)
      printf("Words: %d\n", count);
  }
  printf("Processed %d words\n", count);
  delete[] data;
  while(!strings.empty()) {
    char* str = strings.back();
    strings.pop_back();
    delete[] str;
  }
  
  // a.add("011");
  // a.printWords();
  // a.add("010");
  // a.printWords();
  // a.remove("011");
  // a.printWords();

  // a.add("1");
  // a.add("11");
  // a.add("0");
  // a.add("01");
  
  // a.add("011");
  // a.add("0101");
  // a.add("11111");
  // a.add("0111");
  // a.add("01110");

  // a.add("10");
  // a.add("1100");
  // a.add("11");
  
  // a.remove("0101");
  // a.remove("11111");
  // a.remove("0111");
  // a.remove("01110");

  a.printDot("dot.dot");
  a.printWords();
}
