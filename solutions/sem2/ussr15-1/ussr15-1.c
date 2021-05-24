#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

void SIGRTMINHandler(int signo, siginfo_t *info, void *context) {
    //printf("SIGRTMIN\n");
    //printf("%d\n", (uint32_t) info->si_value.sival_int);
    uint32_t x = (uint32_t)info->si_value.sival_int;
    if (x == 0)
        exit(0);
    union sigval s;
    --x;
    s.sival_int = (uint32_t)x;
    sigqueue(info->si_pid, SIGRTMIN, s);

    fflush(stdout);

}


int main () {
      struct sigaction handler;
  memset(&handler, 0, sizeof(handler));
  handler.sa_handler = (void*)SIGRTMINHandler;
  handler.sa_flags = SA_RESTART | SA_SIGINFO;
  sigaction(SIGRTMIN, &handler, NULL);
  //printf("pid: %d\n", getpid());
  while (1) {
      pause();
  }
    return 0;
}