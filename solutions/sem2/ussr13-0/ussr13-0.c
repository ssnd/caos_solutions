#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <zconf.h>
#include <stdlib.h>

int main(int argc, char * argv[]) {
  char *cmd = argv[1];
  char *input_file = argv[2];

  int in = open(input_file, O_RDONLY);
  dup2(in, 0);
  close(in);

  int pipe_fds[2];
  pipe(pipe_fds);

  pid_t pid = fork();

  if (pid == 0) {
    dup2(pipe_fds[1], 1);
    close(pipe_fds[1]);
    execlp(cmd, cmd, NULL);
    exit(1);
  } else {
    close(pipe_fds[1]);
    uint64_t total_count = 0;
    uint64_t c;
    char buff[4096];
    while ((c = read(pipe_fds[0], buff, sizeof(buff))) > 0) {
      total_count+= c;
    }
    waitpid(pid, 0,0);
    printf("%"PRIu64"\n", total_count);
  }

}