#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

volatile sig_atomic_t counter = 0;
volatile sig_atomic_t is_running = 1;

void SIGPIPEHandler() {
  is_running = 0;
}

int main (int argc, char* argv[]) {
  char * fifo_name = argv[1];
  char * p;
  size_t n = strtol(argv[2], &p, 10);

  mkfifo(fifo_name, 0666);
  int dest_pid;
  scanf("%d", &dest_pid); // NOLINT
  kill(dest_pid, SIGHUP);

  struct sigaction sigpipe_handler;
  memset(&sigpipe_handler, 0, sizeof(sigpipe_handler));
  sigpipe_handler.sa_handler = SIGPIPEHandler;
  sigpipe_handler.sa_flags = SA_RESTART;
  sigaction(SIGPIPE, &sigpipe_handler, NULL);

  char buff[4096];
  memset(buff, '\0', 4096);
  char num[6];
  int len = 0;
  for (size_t i = 0; i <= n; ++i) {
      memset(num, '\0', 5);
      snprintf(num, 5, "%ld", i);
      for (size_t j = 0; j < strlen(num); ++j)
        buff[len++] = num[j];

      buff[len++] = (i == n) ? '\0' : ' ';
  }


  int written_len = 0 ;
  int fd = open(fifo_name, O_WRONLY);

  while (is_running) {

    char ch = buff[written_len++];
    if (' ' == ch) {
      ++counter;
    }
//    printf("writing %c\n", ch);
    write(fd, &ch, 1);

    if (written_len == len-1) {
      is_running = 0;
    }
  }

  close(fd);

  printf("%d\n", counter);
  return 0;
}