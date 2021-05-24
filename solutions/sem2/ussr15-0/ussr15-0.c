#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

volatile char ** ptr;

void read_line(const char * file_name) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    fp = fopen(file_name, "r");
    getline(&line, &len, fp);
    printf("%s", line);
    fflush(stdout);
}

void handler(int signum, siginfo_t * info, void * context) {
    int x = signum - SIGRTMIN;
    //printf("x=%d\n", signum - SIGRTMIN);
    if (x > 0) {
        read_line((const char *)ptr[x]);
    }
    if (x==0) exit(0);
}



int main(int argc, char * argv[]) {
    ptr = (volatile char **)argv;
    struct sigaction sigpipe_handler;
    memset(&sigpipe_handler, 0, sizeof(sigpipe_handler));
  sigpipe_handler.sa_handler = (void*)handler;
  sigpipe_handler.sa_flags = SA_SIGINFO | SA_RESTART;
  for (size_t i = 1; i <= 64; ++i)
    sigaction(i, &sigpipe_handler, NULL);
  //printf("mypid: %d\n", getpid());

    while (1) {
        pause();
    }
}