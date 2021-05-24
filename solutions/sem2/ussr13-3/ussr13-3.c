//
// Created by Sandu Kiritsa on 21.02.2021.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
  for (size_t i = 1; i < argc; ++i) {
    char * cmd_txt = argv[i];
    if (argc == i+1){
      execlp(cmd_txt,cmd_txt,NULL);
      exit(1);
    }

    int pipe_pair[2];
    pipe(pipe_pair);
    pid_t pid = fork();
    if (-1==pid) {
      perror("fork");
      exit(1);
    }

    if (0 == pid) {
      dup2(pipe_pair[1],1);
      close(pipe_pair[1]);
      execlp(cmd_txt,cmd_txt,NULL);
      exit(1);
    } else {
      close (pipe_pair[1]);
      dup2(pipe_pair[0], 0);
      close(pipe_pair[0]);
      waitpid(pid, 0, 0);
    }

  }

  return 0;
}