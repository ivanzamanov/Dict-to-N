#ifndef __WORD_H__
#define __WORD_H__

struct word {
word(const char* const source, int start, int length)
:source(source),start(start),length(length) { }
  // Non-owning pointer to actual data
  const char* const source;
  // Offset of the start of this word
  const int start;
  // This word's length
  const int length;

  char operator[](int i) {
    return source[start + i];
  }
};

#endif
