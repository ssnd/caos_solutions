//
// Created by Sandu Kiritsa on 02.12.2020.
//


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char* argv[]) {
  ssize_t N = strtol(argv[1], NULL, 10);

  for (ssize_t i = N; i > 0; --i) {
    pid_t pid = fork();
    if (pid == 0) { // if you're in a child proess, spawn a new one
      continue;
    }
    // else
    fflush(stdout);
    printf(i < N ? "%zd " : "%zd\n", i);
    wait(NULL);
    exit(0);
  }
    return 0;

}