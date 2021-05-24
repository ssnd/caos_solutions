#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//#define DEBUG 1

#ifdef DEBUG
#define dbg printf
#else
#define dbg
#endif

int server_fd;
int epoll_fd;

volatile sig_atomic_t is_running = 1;
volatile sig_atomic_t epoll_waiting = 0;
const size_t max_connections = 256;

void safe_exit(int code) {
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    close(epoll_fd);
    exit(code);
}


int create_and_bind(const char * host, uint16_t port) {

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (-1 == sock_fd) {
        perror("socket");
        safe_exit(1);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = inet_addr(host),
        .sin_port = htons(port)
    };

    if (bind(sock_fd, (struct sockaddr * ) & addr, sizeof(struct sockaddr_in))) {
        perror("bind");
        safe_exit(1);
    }

    if (listen(sock_fd, max_connections) != 0) {
        perror("listen");
        safe_exit(1);
    }
    dbg("server listening on port %d\n", port);
    return sock_fd;

}

void sigterm_handler() {
  _exit(0);
}

void setup_sighandler() {
    struct sigaction sig_handler;
    memset(&sig_handler, 0, sizeof(sig_handler));
    sig_handler.sa_handler = sigterm_handler;
    sig_handler.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &sig_handler, NULL);
}

void add_epollin_event(int epoll_fd, int target_fd) {
    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.data.fd = target_fd;
    event.events = EPOLLIN;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, target_fd, &event);
}


void accept_incoming_connection(int epoll_fd, int conn_fd) {
    // accept connection from client
    int client_fd = accept(conn_fd, NULL, NULL);
    if (-1 == client_fd) {
        perror("accept");
        safe_exit(1);
    }

    // make non-blocking
    int flags = fcntl(client_fd, F_GETFL);
    fcntl(client_fd, F_SETFL, flags|O_NONBLOCK);

    add_epollin_event(epoll_fd, client_fd);
    dbg("accepting incoming connection\n");
}

char buff[8192];
char output[8192];
const int to_upper = 'A' - 'a';
void process_connection(int conn_fd) {
    dbg("processing connection\n");
    memset(buff, 0, 8192);
    memset(output, 0, 8192);
    int count = read(conn_fd, buff, sizeof(buff));
    if (count == -1) {
        perror("read");
        safe_exit(1);
    }

    if (strlen(buff) == 0) {
        shutdown(conn_fd, SHUT_RDWR);
        close(conn_fd);
        return;
    }
    dbg("read %d chars:\n%s\n", strlen(buff), buff);

    for (size_t i = 0; i < strlen(buff); ++i) {
        if (buff[i] >= 'a' && buff[i] <= 'z') {
            output[i] += buff[i] + to_upper;
        } else {
            output[i] = buff[i];
        }
    }
    dbg("upper version: %s\n", output);

    write(conn_fd, output, strlen(output));
}


int main(int argc, char * argv[]) {
    dbg("starting up\n");
    setup_sighandler();
    uint16_t port = strtol(argv[1], NULL, 10);

    server_fd = create_and_bind("127.0.0.1", port);
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create");
        safe_exit(1);
    }


    // make the server socket non-blocking
    int flags = fcntl(server_fd, F_GETFL);
    fcntl(server_fd, F_SETFL, flags|O_NONBLOCK);
    add_epollin_event(epoll_fd, server_fd);

    int connections;
    struct epoll_event current_event;
    struct epoll_event incoming_events[max_connections];
    dbg("starting the main event loop\n");

    // main event loop
    while (is_running) {
        epoll_waiting = 1;
        connections = epoll_wait(epoll_fd, incoming_events, max_connections, -1);
        epoll_waiting = 0;
        if (connections == -1) {
            perror("epoll_wait");
            safe_exit(1);
        }

        for (size_t i = 0; i < connections; ++i) {
            current_event = incoming_events[i];
            if (current_event.data.fd == server_fd) {
                accept_incoming_connection(epoll_fd, server_fd);
                continue;
            }

            if (current_event.events & EPOLLIN) {
                int client_fd = current_event.data.fd;
                process_connection(client_fd);
                continue;
            }
        }

    }

    safe_exit(0);
    return 0;

}