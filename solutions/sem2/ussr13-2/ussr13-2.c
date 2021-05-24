#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void find_err_type(char* input, size_t len, int * lines, char * searching_for) {
  if( ! strstr(input, searching_for)) {
    return;
  }

  int line_read = 0;

  for (size_t i = 0; i < len; ++i ) {
    if (input[i] == ':' && line_read == 0) {
      char line_no_buff[4096];
      int cnt = 0;
      line_read = 1;
      while (input[i++] != ':') {
        line_no_buff[cnt++] = input[i];
        i++;
      }
      long no = strtol(line_no_buff, NULL, 10);
      lines[no] = 1;
    }
  }

}

int main(int argc, char * argv[]) {
  int pipe_fds[2];
  pipe(pipe_fds);
  int line_warnings[4096];
  int line_errors[4096];
  memset(line_warnings, 0, sizeof(int) * 4096);
  memset(line_errors, 0, sizeof(int) * 4096);

  pid_t pid = fork();

  if (0 == pid) {
    dup2(pipe_fds[1], 2);
    close(pipe_fds[1]);
    execlp("gcc", "gcc", argv[1], NULL);
    exit(1);
  } else {
    close(pipe_fds[1]);
    dup2(pipe_fds[0], 0);
    close(pipe_fds[0]);
    char c;
    char buff[4096];
    size_t count = 0;
    while ((c = getchar()) != EOF) {
      buff[count++] = c;
      if (c == '\n') {
        find_err_type(buff, count, line_errors, "error");
        find_err_type(buff, count, line_warnings, "warning");
        count = 0;
        memset(buff, '\0', 4096);
      }
    }

    size_t total_warning_lines = 0;
    size_t total_error_lines = 0;
    for (size_t i = 0; i < 4096; ++i) {
      if (line_errors[i] != 0)
        total_error_lines++;

      if (line_warnings[i] != 0)
        total_warning_lines++;

    }

    printf("%zu\n%zu\n", total_error_lines, total_warning_lines);
  }
}