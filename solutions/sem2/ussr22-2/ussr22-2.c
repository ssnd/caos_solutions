#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct header {
    unsigned short id;
    unsigned char rd: 1;
    unsigned char tc: 1;
    unsigned char aa: 1;
    unsigned char opcode: 4;
    unsigned char qr: 1;
    unsigned char rcode: 4;
    unsigned char z: 3;
    unsigned char ra: 1;
    unsigned short qdcount;
    unsigned short ancount;
    unsigned short nscount;
    unsigned short arcount;
} dns_header_t;

typedef struct {
    unsigned short qtype;
    unsigned short qclass;
} dns_question_t;

typedef struct {
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
} rdata_t;


void dns_format(char *src, char *dest) {
  int pos = 0;
  int len = 0;
  int i;
  strcat(src, ".");
  for (i = 0; i < (int) strlen(src); ++i) {
    if (src[i] == '.') {
      dest[pos] = i - len;
      ++pos;
      for (; len < i; ++len) {
        dest[pos] = src[len];
        ++pos;
      }
      len++;
    }
  }

  dest[pos] = '\0';
}

const char *dns_server = "8.8.8.8";

void resolve_dns(char *addr) {
  char buff[65536];

  dns_question_t *qflags = NULL;
  dns_header_t *dns_header = NULL;
  char *qname;

  size_t shift = 0;
  dns_header = (dns_header_t *) &buff;
  dns_header->id = (uint32_t) htons(getpid());
  dns_header->qr = 0;
  dns_header->opcode = 0;
  dns_header->aa = 0;
  dns_header->tc = 0;
  dns_header->rd = 1;
  dns_header->ra = 0;
  dns_header->z = 0;
  dns_header->rcode = 0;
  dns_header->qdcount = htons(1);
  dns_header->ancount = 0x0000;
  dns_header->nscount = 0x0000;
  dns_header->arcount = 0x0000;

  shift += sizeof(dns_header_t);
  qname = (char *) &buff[shift];
  char *qname_1 = malloc(sizeof(char) * (strlen(addr)));
  strcpy(qname_1, addr);
  dns_format(qname_1, qname);
  shift += strlen(qname) + 1;
  qflags = (dns_question_t *) &buff[shift];
  qflags->qtype = htons(0x0001);
  qflags->qclass = htons(0x0001);
  shift += sizeof(dns_question_t);

  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(53);
  inet_pton(AF_INET, dns_server, &(servaddr.sin_addr));
  connect(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  write(sock_fd, (char *) buff, shift);
  read(sock_fd, (unsigned char *) buff, 65536);
  dns_header = (dns_header_t *) &buff;
  shift = sizeof(dns_header_t);
  qname = &buff[shift];
  shift += strlen((const char *) qname) +
           sizeof(dns_question_t) + 1; // skip the parts we don't need
  for (size_t k = 0; k < ntohs(dns_header->ancount); ++k) {
    shift += 2;
    rdata_t *rrflags = (rdata_t *) &buff[shift];
    shift += sizeof(rdata_t) - 2;
    if (ntohs(rrflags->type) == 1) { // assume it's always one
      for (size_t i = 0; i < ntohs(rrflags->data_len); ++i) {
        printf("%d", (uint8_t) buff[shift + i]);
        if (i < ntohs(rrflags->data_len) - 1) {
          printf(".");
        }
      }
      printf("\n");
    }
    shift+=ntohs(rrflags->data_len);
  }
}


int main() {
  char buff[1024];
  while (fgets(buff, sizeof(buff), stdin) != NULL) {
    buff[strlen(buff) - 1] = '\0';
    resolve_dns(buff);
  }
}