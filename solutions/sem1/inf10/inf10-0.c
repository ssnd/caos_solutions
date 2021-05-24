#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main(int argc, char *argv[]) {
  char* filename = argv[1];
  char* pattern= argv[2];
  int f = open(filename, O_RDONLY);
  struct stat st;
  fstat(f, &st);
  if (f == -1 || st.st_size == 0) {
    close(f);
    exit(0);
  }

  char* ftext = (char*)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, f, 0);

  char* pointer = ftext;
  while ((pointer = strstr(pointer, pattern)) != NULL) {
    size_t shift = pointer - ftext;
    printf("%lu ", shift);
    ++pointer;
  }

  printf("\n");

  munmap(ftext, st.st_size);
  close(f);

  return 0;

}
