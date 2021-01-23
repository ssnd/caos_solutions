#include <stdio.h>
#include <stdint.h>
#include <string.h>

void DeleteNChars(char* path, size_t n, size_t index) {
  size_t i = 0;
  while (*(path + i+n) != '\0') {
    if ( i < index ) {
      ++i;
      continue;
    }

    *(path + i) = *(path + i+n);
    ++i;
  }

  *(path+i) = '\0';

};
extern void normalize_path(char * path) {
  int i = 0;

  while (*(path + i) != '\0') {

    if (*(path + i) == '/' && *(path + i + 1) == '.' && *(path + i + 2) == '.') {
      DeleteNChars(path, 3, i + 1);
      size_t counter = 1;
      --i;
      while (*(path + i) != '/' && i != 0) {
        ++counter;
        --i;
      }
      DeleteNChars(path, counter, i);

    }

    if (*(path + i) == '/' && *(path + i + 1) == '/') {
      DeleteNChars(path, 1, i);
      --i;

    }

    if (i>0 && *(path+i) == '.' && *(path+i+1) == '/') {
      DeleteNChars(path, 2, i);
    }

    ++i;
  }
}
