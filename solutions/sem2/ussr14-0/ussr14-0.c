#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>


volatile sig_atomic_t counter = 0;
volatile sig_atomic_t exit_trigger = 0;

void SIGINTHandler() {
    counter++;
}

void SIGTERMHandler() {
    exit_trigger = 1;
}


int main () {

    struct sigaction sigint_handler;
    memset(&sigint_handler, 0, sizeof(sigint_handler));
    sigint_handler.sa_handler = SIGINTHandler;
    sigint_handler.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sigint_handler, NULL);

    struct sigaction sigterm_handler;
    memset(&sigterm_handler, 0, sizeof(sigterm_handler));
    sigterm_handler.sa_handler = SIGTERMHandler;
    sigterm_handler.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &sigterm_handler, NULL);

    printf("%d\n", getpid());
    fflush(stdout);

    while (!exit_trigger) {
        pause();
    }
    printf("%d\n", counter);

	return 0;

}