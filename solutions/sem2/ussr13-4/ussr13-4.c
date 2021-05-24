#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {

    int total = argc - 1;
    int pid_arr[total];

    int pipe1_fd[2];
    int pipe2_fd[2];
    pipe2(pipe1_fd, O_NONBLOCK);
    pipe2(pipe2_fd, O_NONBLOCK);

    size_t i = 0;

    while (i < total) {
      pid_arr[i] = fork();
      if (-1==pid_arr[i]) {
        perror("fork");
        exit(1);
      }

      if (0==pid_arr[i]) {
        if (i%2==0) {
          if (i<total-1) {
            dup2(pipe1_fd[1], 1);
          }

          if (i>0) {
            dup2(pipe2_fd[0],0);
          }
          execlp(argv[i+1],argv[i+1], NULL);
          exit(1);
        } else {
          if (i<total-1) {
            dup2(pipe2_fd[1],1);
          }
          dup2(pipe1_fd[0],0);
          execlp(argv[i+1],argv[i+1], NULL);
          exit(1);
        }


      } else {
        waitpid(pid_arr[i],0,0);
        ++i;
      }
    }

    close(pipe1_fd[0]);
    close(pipe1_fd[1]);
    close(pipe2_fd[0]);
    close(pipe2_fd[1]);

    for (i = 0; i < total; ++i) {
        waitpid(pid_arr[i], 0, 0);
    }
    return 0;
};