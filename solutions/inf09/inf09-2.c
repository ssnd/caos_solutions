#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>

#include <fcntl.h>
#include <stdlib.h>

uint32_t kELFHeader = 0x464c457f;

uint8_t CheckIfELF(FILE * fp) {
  uint32_t header;
  fread(&header, sizeof(uint32_t), 1, fp);

  if (header == kELFHeader) {
    return 1;
  }

  return 0;
}

char* GetShebangExecutor(FILE* fp) {
  int ch;
  char* exec = (char*) calloc(sizeof(char),  512);
  int i = 0;
  uint8_t shebang = 0;
  while ((ch = fgetc(fp)) != EOF) {

    exec[i] = ch;

    if (i == 0) {
      ++i; continue;
    }

    if (exec[i] == '!' && exec[i-1] == '#') {
      shebang = 1;
      break;
    }
    ++i;
  }

  if (shebang == 0)
    return NULL;

  i = 0;
  while ((ch = fgetc(fp)) != EOF) {
    if (ch == ' ')
      continue;

    exec[i] = (char) ch;
    if (ch == '\n')
      break;

    ++i;
  }

  if (i == 0)
    return NULL;

  exec[i] = '\0';
  return exec;
}

uint8_t CheckIfFileIsExecutable(char* filename) {
  FILE* fp = fopen(filename, "rb");
  struct stat st;
  if (fp == NULL) {
    return 1;
  }

  int st_status = fstat(fileno(fp), &st);

  if ((st.st_mode & S_IXUSR) != S_IXUSR || st_status == -1) {
    return 1;
  }

  if (CheckIfELF(fp) == 1 && (st.st_mode & S_IXUSR) == S_IXUSR ){
    return 1;
  }

  if (CheckIfELF(fp) == 0) {
    fp = fopen(filename, "rb");
    char *exec = GetShebangExecutor(fp);

    if (exec == NULL)
      return 0;

    fp = fopen(exec, "rb");
    free(exec);

    if (fp == NULL)
      return 0;

    st_status = fstat(fileno(fp), &st);

    if (CheckIfELF(fp) == 1 && st_status != -1 && (st.st_mode & S_IXUSR) == S_IXUSR) {
      return 1;
    } else {
      return 0;
    }
  }

  return 0;
}

int main(int argc, char* argv[]) {
  size_t kMaxSize = 2048;
  int ch;
  char file[kMaxSize];
  size_t i = 0;

  while ((ch = getchar()) != EOF) {
    if (ch == '\n') {
      file[i] = '\0';
      if (i==0)
        continue;
      i = 0;
      if (CheckIfFileIsExecutable(file) == 0) {
        printf("%s ", file);
      }
      continue;
    }
    file[i] = (char) ch;
    ++i;
  }

  printf("\n");

  return 0;
}