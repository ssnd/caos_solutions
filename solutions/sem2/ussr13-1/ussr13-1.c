#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char * argv[]) {
  char * cmd1 = argv[1];
  char * cmd2 = argv[2];

  int pipe_fds[2];
  pipe(pipe_fds);

  pid_t pid = fork();

  if (0 == pid) {
    dup2(pipe_fds[1], 1);
    close(pipe_fds[1]);
    execlp(cmd1, cmd1, NULL);
    exit(1);
  } else {
    close(pipe_fds[1]);
    dup2(pipe_fds[0], 0);
    waitpid(pid, 0,0);
    execlp(cmd2, cmd2, NULL);
    exit(1);
  }

}