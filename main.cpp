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

#define PROGRESS

struct FileData {
  char* data;
  int size;
};

extern void printPools();
typedef std::vector<char*> StringVector;
void readStrings(char* data, int size, StringVector& vec);

void processData(Autom& autom, FileData data, void (*func)(Autom&,const char*, const char*)) {
  StringVector strings;
  readStrings(data.data, data.size, strings);

  StringVector::iterator it = strings.begin();
  int count = 0;

#ifdef PROGRESS
  int total = strings.size() / 2;
  int progress = 0;
  fprintf(stderr, "Progress: %2d%%", progress);
#endif

  fflush(stdout);
  while(it != strings.end()) {
    const char* key = *it;
    it++;
    const char* valueStr = *it;
    func(autom, key, valueStr);
    it++;

    count++;
#ifdef PROGRESS
    int oldProgress = progress;
    progress = ((double)count / (double)total) * 100;
    if(progress != oldProgress) {
      fprintf(stderr, "\rProgress: %2d%%", progress);
      fflush(stdout);
    }
#endif
  }
  fprintf(stderr, "\nProcessed %d words\n", count);
  fprintf(stderr, "Total states %d\n", autom.getStateCount());
  fprintf(stderr, "Final states %d\n", autom.getFinalStateCount());
  fprintf(stderr, "Total transitions %d\n", autom.getTransitionCount());

  while(!strings.empty()) {
    char* str = strings.back();
    strings.pop_back();
    delete[] str;
  }
}

void addFunc(Autom& autom, const char* key, const char* val) {
  int value = atoi(val);
  autom.add(key, value);
}

void removeFunc(Autom& autom, const char* key, const char* val) {
  autom.remove(key);
}

void addAll(Autom& autom, FileData data) {
  processData(autom, data, addFunc);
}

void removeAll(Autom& autom, FileData data) {
  processData(autom, data, removeFunc);
}

void doWork(FileData all, FileData toRemove, FileData removed) {
  Autom removedAutom;
  Autom removeAutom;
  fprintf(stderr, "Building all\n");
  addAll(removedAutom, removed);
  fprintf(stderr, "Building for remove\n");
  addAll(removeAutom, all);
  fprintf(stderr, "Removing\n");
  removeAll(removeAutom, toRemove);
  fprintf(stderr, "Comparing\n");
  fprintf(stderr, "Isomorphic: %d\n", removedAutom.isIsomorphic(removeAutom));
  removeAutom.printWords();
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
  fprintf(stderr, "Equal: %d\n", a.isIsomorphic(a));
}

void test2() {
  Autom a;
  a.add("aa", 10);
  a.add("aaa", 7);
  a.add("b", 7);
  a.add("baa", 9);
  a.printDot("/tmp/temp.dot");
  fprintf(stderr, "Equal: %d\n", a.isIsomorphic(a));
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
  fprintf(stderr, "Equal: %d\n", a.isIsomorphic(a));
}

bool readData(FileData& dest, const char* file) {
  int fd = open(file, O_RDONLY);
  struct stat fileStat;

  if(fd < 0) {
    fprintf(stderr, "Cannot open file %s\n", file);
    return false;
  }

  if(fstat(fd, &fileStat) < 0) {
    fprintf(stderr, "Cannot stat file %s\n", file);
    return false;
  }

  long int size = fileStat.st_size;
  char* data = new char[size + 1];
  read(fd, data, size);
  data[size] = 0;
  dest.data = data;
  dest.size = size;
  return true;
}

int main(int argc, const char** argv) {
  argc = 4;
  if(argc < 4) {
    fprintf(stderr, "Not enough input files specified\n");
    return 1;
  }
  FileData all, toRemove, removed;
  bool success;
  success = readData(all, argv[1]);
  success = success && readData(toRemove, argv[2]);
  success = success && readData(removed, argv[3]);
  if(success) {
    // test1();
    // test2();
    // test3();
    doWork(all, toRemove, removed);
    delete all.data;
    delete toRemove.data;
    delete removed.data;
  }
}

bool isWhitespace(char c) {
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
    if(i == 0)
      continue;
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
    fprintf(stderr, "%s\n", *it);
    ++it;
  }
}
