#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile sig_atomic_t done = 0;

typedef struct icmp_packet {
  struct icmphdr header;
  char msg[64 - sizeof(struct icmphdr)];
} icmp_packet_t;

uint16_t checksum(void *b, int len) {
    uint16_t *buf = b;
    uint sum = 0;
    uint16_t result;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(unsigned char*)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void alarm_handler() {
  done = 1;
}

void setup_ping(int fd, struct sockaddr *conn, size_t conn_size,  size_t interval) {
  bind(fd, conn, conn_size);

  struct icmp_packet packet;
  int i;
  int count = 0;
  size_t resp_count = 0;

  while (!done) {
    bzero(&packet, sizeof(packet));
    packet.header.type = ICMP_ECHO;
    packet.header.un.echo.id = getpid();
    for (i = 0; i < sizeof(packet.msg) - 1; ++i) {
      packet.msg[i] = i + '0';
    }

    packet.msg[i] = 0;
    packet.header.un.echo.sequence = count++;
    packet.header.checksum = checksum(&packet, sizeof(packet));
    sendto(fd, &packet, sizeof(packet), 0, conn, conn_size);
    struct sockaddr_in repl;

    uint repl_size;
    recvfrom(fd, &packet, sizeof(packet), 0, (struct sockaddr *) &repl, &repl_size);
    ++resp_count;
    usleep(interval);
  }

  printf("%ld\n", resp_count);
}

int main(int argc, char *argv[]) {
  in_addr_t addr = inet_addr(argv[1]);
  in_port_t port = htons(0);

  struct sigaction alrm;
  alrm.sa_handler = alarm_handler;
  sigaction(SIGALRM, &alrm, NULL);

  size_t timeout = strtol(argv[2], NULL, 10);
  size_t interval = strtol(argv[3], NULL, 10);

  if (0 == timeout) {
    printf("0\n");
    exit(0);
  }
  alarm(timeout);

  struct sockaddr_in ip_connection = {
      .sin_family=AF_INET, .sin_port=port, .sin_addr=addr
  };
  int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (fd<0) {
    perror("socket_open");
    exit(1);
  }

  setup_ping(fd, (struct sockaddr *) &ip_connection, sizeof(ip_connection), interval);

  close(fd);

  return 0;
}