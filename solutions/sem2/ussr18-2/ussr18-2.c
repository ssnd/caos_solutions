//
// Created by Sandu Kiritsa on 24.03.2021.
//
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

//#define DEBUG 1

typedef struct payload_struct_ {
    int socketpair_fd[2];
    int n;
} payload_struct;

volatile sig_atomic_t is_running = 1;

#ifdef DEBUG
#define dbg printf
#else
#define dbg
#endif

enum game_state {
    SENDING, WAITING
};
// this is the thread that starts the game
static void * thread_ping(void * arg) {
    const int sockepair_index = 0;
    const char * thread_name = "ping";

    payload_struct payload = *((payload_struct*) arg);
    int value = payload.n;
    int socket_fd = payload.socketpair_fd[sockepair_index];

    enum game_state state = SENDING;
    while (1) {
        // check if done
        if (!is_running) break;

        if (state == SENDING) {
            value -= 3;
            if (value == 0 || value > 100) {
                break;
            }
            printf("%d\n", value);

            dbg("%s sending value %d\n", thread_name, value);
            write(socket_fd, &value, sizeof(int));
            state = WAITING;
            continue;
        }

        if (state == WAITING) {
            read(socket_fd, &value, sizeof(int));
            dbg("%s received value %d\n", thread_name,  value);
            state = SENDING;
            continue;
        }
    }
    // should be done here
    if (is_running) {
        is_running = 0;
        printf("%d\n", value);
        dbg("%s is finishing the game\n", thread_name);
        write(socket_fd, &value, sizeof(int));
    }
    dbg("ping thread done, n=%d\n", payload.n);
    return NULL;
}


static void * thread_pong(void * arg) {
    const char * thread_name = "pong";
    const int sockepair_index = 1;

    payload_struct payload = *((payload_struct*) arg);

    int socket_fd = payload.socketpair_fd[sockepair_index];

    enum game_state state = WAITING;
    int value;
    while (1) {
        if (!is_running) break;
        if (state == WAITING) {
            read(socket_fd, &value, sizeof(int));
            dbg("%s received value %d\n", thread_name,  value);
            state = SENDING;
            continue;
        }
        // this is always true, keeping here for readability
        if (state == SENDING) {
            value += 5;
            if (value == 0 || value > 100) {
                break;
            }
            printf("%d\n", value);

            write(socket_fd, &value, sizeof(int));
            dbg("%s sending value %d\n", thread_name, value);
            state = WAITING;
            continue;
        }
    }
    if (is_running) {
        is_running = 0;
        printf("%d\n", value);
        dbg("%s is  finishing the game\n",thread_name);
        write(socket_fd, &value, sizeof(int));
    }
    dbg("pong thread done, n=%d\n", payload.n);
    return NULL;
}


int main(int argc, char *argv[]) {

#ifdef DEBUG
    int n = 15;
#else
    int n = strtol(argv[1], NULL, 10);
#endif
    payload_struct payload;
    payload.n = n;
    payload_struct * payload_addr = &payload;
    socketpair(AF_UNIX, SOCK_STREAM, 0, payload.socketpair_fd);
    dbg("%d %d\n", payload.socketpair_fd[1], payload.socketpair_fd[0]);
    pthread_t ping, pong;
    pthread_create(&ping, NULL, thread_ping, (void *) payload_addr);
    pthread_create(&pong, NULL, thread_pong, (void *) payload_addr);
    pthread_join(ping, NULL);
    pthread_join(pong, NULL);
    return 0;
}