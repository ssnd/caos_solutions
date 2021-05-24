//
// Created by Sandu Kiritsa on 02.12.2020.
//


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char* argv[]) {
  char buffer[4096];
  fflush(stdin);
  int counter = 0;
  while (1) {
    pid_t pid = fork();
    if (pid == 0) {
      if (scanf("%s", buffer) != EOF) {
        exit(1);
      } else {
        exit(0);
      }
    }
    int status;
    wait(&status);
    if (WEXITSTATUS(status)) {
      ++counter;
    } else {
      break;
    }
  }
  printf("%d\n", counter);
  return 0;
}