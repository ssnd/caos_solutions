#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>


int main(int argc, char *argv[]) {
    char *host_name = argv[1];
    char *path = argv[2];
    signal(SIGPIPE, SIG_IGN);

    struct addrinfo addr_hints = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM };
    struct addrinfo *addr_result = NULL;
    getaddrinfo(host_name, "http", &addr_hints, &addr_result);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (0!=connect(sock, addr_result->ai_addr, addr_result->ai_addrlen)) {
        perror("connect"); exit(1);
    }
    char request[4096];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\n"
             "Host: %s\n"
             "Connection: close\n"
             "\n",
             path, host_name);
    write(sock, request, strnlen(request, sizeof(request)));
    FILE* in = fdopen(sock, "r");
    char minibuf[65536];
    bool headers_completed = 0;
    while (fgets(minibuf, sizeof(minibuf), in)) {
        if (strstr(minibuf, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">") != NULL) {
          break;
        }

        if (0==strcmp(minibuf, "\n") || 0==strcmp(minibuf,"\r\n")) {
            headers_completed = 1;
            continue;
        }
        if (headers_completed) {
            printf("%s", minibuf);
        }
    };
    fclose(in);
    close(sock);
}