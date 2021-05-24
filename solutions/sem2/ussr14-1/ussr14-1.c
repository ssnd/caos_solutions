#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>


volatile sig_atomic_t counter = 0;
volatile sig_atomic_t exit_trigger = 0;

void SIGUSR1Handler() {
  counter++;
  printf("%d\n", counter);
  fflush(stdout);
}

void SIGUSR2Handler() {
  counter*=-1;
  printf("%d\n", counter);
  fflush(stdout);
}

void SafeExitHandler() {
  exit_trigger = 1;
}

int main () {

    struct sigaction sigusr1_handler;
    memset(&sigusr1_handler, 0, sizeof(sigusr1_handler));
    sigusr1_handler.sa_handler = SIGUSR1Handler;
    sigusr1_handler.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sigusr1_handler, NULL);

    struct sigaction sigusr2_handler;
    memset(&sigusr2_handler, 0, sizeof(sigusr2_handler));
    sigusr2_handler.sa_handler = SIGUSR2Handler;
    sigusr2_handler.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &sigusr2_handler, NULL);

    struct sigaction exit_handler;
    memset(&exit_handler, 0, sizeof(exit_handler));
    exit_handler.sa_handler = SafeExitHandler;
    exit_handler.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &exit_handler, NULL);
    sigaction(SIGINT, &exit_handler, NULL);

    printf("%d\n", getpid());
//    int c;
    scanf("%d", &counter);
//    counter = (sig_atomic_t) c;
    fflush(stdout);

    while (!exit_trigger) {
        pause();
    }

	return 0;

}