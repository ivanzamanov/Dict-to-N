#include<stdio.h>
#include<cstdlib>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#include<vector>
#include<string>
#include<cstring>
#include<vector>

#include"autom.hpp"

extern void printPools();
typedef std::vector<char*> StringVector;
void readStrings(char* data, int size, StringVector& vec);

void doWork(char* data, int size) {
  Autom a;
  StringVector strings;
  readStrings(data, size, strings);
  StringVector::iterator it = strings.begin();
  int count = 0;
  int progress = 0;
  int total = strings.size() / 2;
  printf("Progress: %2d%%", progress);
  fflush(stdout);
  while(it != strings.end()) {
    const char* key = *it;
    it++;
    const char* valueStr = *it;
    int value = atoi(valueStr);
    a.add(key, value);
    it++;

    count++;
    int oldProgress = progress;
    progress = ((double)count / (double)total) * 100;
    if(progress != oldProgress) {
      printf("\rProgress: %2d%%", progress);
      fflush(stdout);
    }
  }
  printf("\nProcessed %d words\n", count);
  printf("Total states %d\n", a.getStateCount());
  printf("Total transitions %d\n", a.getTransitionCount());
  //  printPools();
  delete[] data;
  while(!strings.empty()) {
    char* str = strings.back();
    strings.pop_back();
    delete[] str;
  }
}

void test3() {
  Autom a;
  a.add("aa", 10);
  a.add("aaa", 7);
  a.add("baa", 9);
  a.add("b", 7);
  a.add("ba", 4);
  a.remove("ba");
  a.remove("baa");
  a.printDot("/tmp/temp.dot");
}

void test2() {
  Autom a;
  a.add("aa", 10);
  a.add("aaa", 7);
  a.add("b", 7);
  a.add("baa", 9);
  a.printDot("/tmp/temp.dot");
}

void test1() {
  Autom a;
  a.add("aa", 10);
  a.add("aaa", 7);
  a.add("baa", 9);
  a.add("b", 7);
  a.add("ba", 4);
  a.remove("ba");
  //  a.remove("baa");
  a.printDot("/tmp/temp.dot");
}

int main(int argc, const char** argv) {
  argc = 2;
  argv[1] = "io-data.txt";
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
  test1();
  test2();
  test3();
  //  doWork(data, size);
}

inline bool isWhitespace(char c) {
  return c == 0 || c == '\n' || c == ' ' || c == '\t';
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
    char* str = new char[i+1];
    strncpy(str, data + offset, i+1);
    str[i] = 0;
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
