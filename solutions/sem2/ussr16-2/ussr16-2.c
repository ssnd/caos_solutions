#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>


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

const uint32_t ELF = 0x464c457f;

uint8_t is_elf(FILE * file_stream) {
    uint32_t header;
    fread(&header, sizeof(uint32_t), 1, file_stream);

    if (header == ELF) {
        return 1;
    }

    return 0;
}
char* get_shebang(FILE* fp) {
  int ch;
  char* exec = (char*) calloc(sizeof(char),  512);
  int i = 0;
  uint8_t shebang = 0;
  while ((ch = fgetc(fp)) != EOF) {

    exec[i] = ch;

    if (i == 0) {
      ++i; continue;
    }

    if (exec[i] == '!' && exec[i-1] == '#') {
      shebang = 1;
      break;
    }
    ++i;
  }

  if (shebang == 0)
    return NULL;

  i = 0;
  while ((ch = fgetc(fp)) != EOF) {
    if (ch == ' ')
      continue;

    exec[i] = (char) ch;
    if (ch == '\n')
      break;

    ++i;
  }

  if (i == 0)
    return NULL;

  exec[i] = '\0';
  return exec;
}


uint8_t is_executable(const char * file_name) {
    FILE* fd = fopen(file_name, "rb");
    struct stat st;
    if (fd == NULL) {
        return 0;
    }

    int st_status = fstat(fileno(fd), &st);

    if ((st.st_mode & S_IXUSR) != S_IXUSR || st_status == -1) {
        fclose(fd);
        return 0;
    }

    if (is_elf(fd) == 1 && (st.st_mode & S_IXUSR) == S_IXUSR ) {
        fclose(fd);
        return 1;
    }

    if (is_elf(fd) == 0) {
      fd = fopen(file_name, "rb");
      char * exec = get_shebang(fd);

      if (exec == NULL)
        return 0;

      FILE* fp = fopen(exec, "rb");
      free(exec);

      if (fp == NULL)
        return 0;

      st_status = fstat(fileno(fp), &st);

      if (is_elf(fp) == 1 && st_status != -1 && (st.st_mode & S_IXUSR) == S_IXUSR) {
        fclose(fp);
        return 1;
      } else {
        fclose(fp);
        return 0;
      }

    }

    fclose(fd);
    return 0;
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
  send_header(conn_fd, resp_status);
  if (resp_status == OK) {
    file_size = get_file_size(path);
  }
  if (resp_status == OK && is_executable(path)) {
    pid_t pid = fork();
      if (0 == pid) {
        dup2(conn_fd, 1);
        execlp(path, path, NULL);
      } else {
        waitpid(pid, 0, 0);
      }
  } else {
    send_content_length(conn_fd, file_size);
    send_newline(conn_fd);
    if (resp_status == OK)
      send_file_contents(conn_fd, path);
  }
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
  shutdown(sock_fd, SHUT_RDWR);
  close(sock_fd);
  return 0;
}

