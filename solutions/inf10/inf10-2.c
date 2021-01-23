#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "stdio.h"
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "stdint.h"

void GenerateSpiralMatrix(int** arr, int N) {
  int layers = (N+1)/2;
  int counter = 0;

  for ( int i = 0; i < layers; ++i ){
    for (int j = i; j < N - i; ++j) {
      arr[i][j] = ++counter;
    }

    for (int j = i + 1; j < N - i; ++j) {
      arr[j][N-i-1] = ++counter;
    }
    for (int j = N - i - 2; j >= i; --j) {
      arr[N-i-1][j] = ++counter;
    }

    for (int j = N - i - 2; j > i; --j) {
      arr[j][i] = ++counter;
    }
  }
}

void DisplayMatrix(int** arr, int N, int W) {
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j)
      printf("%*d", W, arr[i][j]);
    printf("\n");
  }
}

void WriteMatrixToFile(int** arr, int N, int W, char* file_pointer) {
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      sprintf(file_pointer, "%*d", W, arr[i][j]);
      file_pointer += W;
    }
    sprintf(file_pointer, "\n");
    ++file_pointer;
  }
}

int main(int argc, char *argv[]) {
  uint32_t W = strtol(argv[3], NULL, 10);
  uint32_t N = strtol(argv[2], NULL, 10);
  int f = open(argv[1], O_RDWR | O_CREAT, 0640);

  ftruncate(f, N*N*W+N);

  char * file_pointer = mmap(NULL, N*N*W+N, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0);

  int* arr[N];
  for (int i = 0; i < N; ++i) arr[i] = (int*)malloc(N*sizeof(int));

  GenerateSpiralMatrix(arr, N);

  DisplayMatrix(arr, N, W);

  WriteMatrixToFile(arr, N, W, file_pointer);
  msync(file_pointer, N*N*W+N, MS_SYNC);
  munmap(file_pointer, N*N*W+N);
  close(f);
}