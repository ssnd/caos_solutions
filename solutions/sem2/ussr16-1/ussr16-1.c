#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

volatile sig_atomic_t is_running = 1;


const char * kHTTPRequestEpilogue = "\r\n\r\n";
enum HTTPCodes {OK = 200, NOT_FOUND = 404, FORBIDDEN = 403};

void termination_handler() {
  is_running = 0;
}

int setup_server(char * ip, char * ch_port) {
  int port = strtol(ch_port, NULL, 10);
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (-1 == sock_fd) {
    perror("socket");
    exit(1);
  }

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_addr =  inet_addr(ip),
    .sin_port = htons(port)
  };

  if (bind(sock_fd,(struct sockaddr*) &addr, sizeof(struct sockaddr_in))) {
    perror("bind");
    exit(1);
  }

  if (listen(sock_fd, 5) != 0) {
    exit(1);
  }

//  printf("server listening");

  return sock_fd;
}

enum HTTPCodes get_response_status(const char * file_name) {
  if (access(file_name, F_OK) == -1) {
    return NOT_FOUND;
  }
  return access(file_name, R_OK) != -1 ? OK : FORBIDDEN;
}

size_t get_file_size(const char* file_name) {
    struct stat st;
    lstat(file_name, &st);
    return st.st_size;
}

void send_header(int fd, enum HTTPCodes status) {
  char resp[4096];

  if (status == OK) {
//    printf("ok\n");
    strcpy(resp, "HTTP/1.1 200 OK\r\n");
  }

  if (status == FORBIDDEN) {
//    printf("403\n");
    strcpy(resp, "HTTP/1.1 403 Forbidden\r\n");
  }

  if (status == NOT_FOUND) {
//    printf("404\n");

    strcpy(resp, "HTTP/1.1 404 Not Found\r\n");
  }

  write(fd, resp, strlen(resp));
}

void send_content_length(int fd, size_t size) {
  char resp[4096];
  memset(resp, 0, 4096);
  snprintf(resp, 4096, "Content-Length: %ld\r\n", size);
  write(fd, resp, strlen(resp));
}

void send_newline(int fd) {
  write(fd, "\r\n", 2);
}

void send_file_contents(int conn_fd, const char * file_name) {
  int fd = open(file_name, O_RDONLY);
  char buff[4096];
  size_t size;
  while ((size = read(fd, buff, sizeof(buff))) > 0) {
      write(conn_fd, buff, size);
  }
}

void handle_client_connections(int sock_fd, const char * dir_path, void (*request_handler)(int, const char *,  char*, size_t)) {
  char buff[1024];
  struct sockaddr_in cli;

  socklen_t len = sizeof(cli);
  int conf_fd;

  size_t req_len;
  char request_body[8192];

  while (is_running) {
    memset(request_body, 0 , 8192);
    req_len = 0;

    conf_fd = accept(sock_fd, (struct sockaddr*)&cli, &len);

    if (conf_fd < 0) {
      exit(1);
    }
//
// printf "GET ctest HTTP/1.1\r\nadfadsf\r\nasdfasdf\r\nasdfaf\r\n\r\n" | nc -t 127.0.0.1 8080
//

    while (is_running) {
      memset(buff, 0, sizeof(buff));
      read(conf_fd, buff, sizeof(buff));
      strcpy(request_body+req_len, buff);
      req_len += strlen(buff);
      if (strcmp(request_body+strlen(request_body) - strlen(kHTTPRequestEpilogue), kHTTPRequestEpilogue) == 0) {
        request_handler(conf_fd, dir_path, request_body, strlen(request_body));
        break;
      }
    }

    close(conf_fd);
  }
}

void my_handler(int conn_fd, const char * dir_path, char * request_body, size_t request_len) {
//  printf("my_handler called, body: %s\n", request_body);
  const char* correct_pattern = "GET ";
  if (strncmp(request_body, correct_pattern, strlen(correct_pattern)) != 0) {
    return;
  }

  const char *  file_name = strtok(request_body + strlen(correct_pattern), " ");
  char path[1024];
  memset(path, 0, 1024);
  snprintf(path, 1024, "%s/%s", dir_path, file_name);
//  printf("%s\n", path);
  const enum HTTPCodes resp_status = get_response_status(path);

  size_t file_size = 0;
  if (resp_status == OK) {
    file_size = get_file_size(path);
  }

//  printf("file_name=%s, resp_status=%d, file_size=%ld\n", file_name, resp_status, file_size);

  send_header(conn_fd, resp_status);
  send_content_length(conn_fd, file_size);
  send_newline(conn_fd);

  if (resp_status == OK)
   send_file_contents(conn_fd, path);
}

void setup_sighandlers() {
  struct sigaction sigpipe_handler;
  memset(&sigpipe_handler, 0, sizeof(sigpipe_handler));
  sigpipe_handler.sa_handler = termination_handler;
  sigpipe_handler.sa_flags = SA_SIGINFO;
  sigaction(SIGTERM, &sigpipe_handler, NULL);
  sigaction(SIGINT, &sigpipe_handler, NULL);
}

int main(int argc, char * argv[]) {
  setup_sighandlers();
  int sock_fd = setup_server("127.0.0.1", argv[1]);
  handle_client_connections(sock_fd, argv[2], my_handler);
  close(sock_fd);
  return 0;
}

