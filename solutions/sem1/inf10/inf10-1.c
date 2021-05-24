#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
  char* filename = argv[1];
  int f = open(filename, O_RDONLY);
  struct stat st;
  fstat(f, &st);
  if (f == -1 || st.st_size == 0) {
    close(f);
    exit(0);
  }

  int* ftext = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, f, 0);

  printf("%d ", ftext[0]);

  uint32_t next_p = ftext[1];
  do {
    int v = ftext[next_p / sizeof(int)];
    printf("%d ", v);

  } while (( next_p = ftext[next_p/sizeof(int)+1] ) != 0);

  printf("\n");
  munmap(ftext, st.st_size);
  close(f);

  return 0;

}