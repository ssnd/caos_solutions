#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdlib.h>
volatile sig_atomic_t child_status = 0;

void SIGCHLDHandler(int sign) {
  child_status = 1;
}

void SIGALRMHandler(int s) {
  child_status = 2;

}


int main (int argc, char * argv[]) {
  int timeout = atoi(argv[1]);

  char * file_name = argv[2];

  struct sigaction handler;
  memset(&handler, 0, sizeof(handler));
  handler.sa_handler = (void*)SIGCHLDHandler;
  handler.sa_flags = SA_RESTART;
  sigaction(SIGCHLD, &handler, NULL);

  memset(&handler, 0, sizeof(handler));
  handler.sa_handler = (void*)SIGALRMHandler;
  handler.sa_flags = SA_RESTART;
  sigaction(SIGALRM, &handler, NULL);

  pid_t pid = fork();
  alarm(timeout);


  if (pid == 0) {
      execvp(file_name, argv+2);
      exit(1);
  } else {
    for (;;) {
      if (child_status != 0) {
        break;
      }
    }

    if (child_status == 2) {
      kill(pid, SIGTERM);
      printf("timeout\n");
      exit(2);
    }

    if (child_status == 1) {
      int s;
      waitpid(pid, &s, 0);
      if (WIFSIGNALED(s)) {
        printf("signaled\n");
        exit(1);
      }
      if (WIFEXITED(s)) {
        printf("ok\n");
        exit(0);
      }
    }

  }
}