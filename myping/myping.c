#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>

#define REPLY_TIMEOUT 5
#define REPLY_BUFFER_SIZE 200
#define _PATH_PROCNET_ARP "/proc/net/arp"

/*
   Usage example:

$ ./myping 192.168.40.1
got reply from 192.168.40.1 in 0ms
MAC-address for 192.168.40.1: 00:50:56:c0:00:08

*/

unsigned short icmp_cksum(unsigned char *, int);
void perrorExit(const char *);
int timediffMs(struct timeval);
int timeval_subtract(struct timeval *, struct timeval *, struct timeval *);
int getMACAddress(const char *host, char *mac);

int main(int argc, char *argv[]) {

  struct protoent *proto;
  int fd;
  char buffer[sizeof(struct icmphdr)];
  int buflen;
  int ret;
  struct sockaddr_in destaddr;
  struct icmphdr *hdr;

  fd_set fdset;
  struct timeval resp_time;
  int n;

  socklen_t srcaddrlen;
  struct sockaddr_in srcaddr;
  struct timeval start_time;

  if (argc < 2 || strcmp(argv[1], "--help") == 0) {
    fprintf(stderr, "%s ip-address\n", argv[0]);
    return EXIT_FAILURE;
  }
  //   fprintf(stderr, "PID = %d\n", getpid());

  proto = getprotobyname("icmp");
  if (!proto) {
    fprintf(stderr, "ping: unknown protocol icmp.\n");
    return EXIT_FAILURE;
  }

  fd = socket(AF_INET, SOCK_RAW, proto->p_proto);
  if (fd < 0) {
    if (errno == EPERM || errno == EACCES) {
      errno = 0;
      fd = socket(AF_INET, SOCK_DGRAM, proto->p_proto);
      if (fd < 0)
        perrorExit("socket1");
    } else {
      perrorExit("socket2");
    }
  }

  memset(&destaddr, 0, sizeof(struct sockaddr_in));
  destaddr.sin_family = AF_INET;
  if (inet_pton(AF_INET, argv[1], &destaddr.sin_addr) <= 0) {
    fprintf(stderr, "inet_pton failed for address %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  buflen = sizeof(struct icmphdr);
  hdr = (struct icmphdr *)&buffer;
  hdr->type = ICMP_ECHO;
  hdr->code = 0;
  hdr->un.echo.id = htons(getpid());
  hdr->un.echo.sequence = htons(1);
  hdr->checksum = 0;
  hdr->checksum = icmp_cksum((unsigned char *)buffer, buflen);

  ret = sendto(fd, buffer, buflen, 0, (struct sockaddr *)&destaddr,
               sizeof(struct sockaddr_in));
  if (ret < 0)
    perrorExit("sendto");

  gettimeofday(&start_time, NULL);

  FD_ZERO(&fdset);
  FD_SET(fd, &fdset);
  resp_time.tv_sec = REPLY_TIMEOUT;
  resp_time.tv_usec = 0, n = select(fd + 1, &fdset, NULL, NULL, &resp_time);
  if (n == 0) {
    fprintf(stderr, "timeout");
    return EXIT_FAILURE;
  }
  if (n < 0)
    perrorExit("select");

  srcaddrlen = sizeof(srcaddr);
  memset(&srcaddr, 0, srcaddrlen);

  char buffer_reply[REPLY_BUFFER_SIZE] = {0};

  n = recvfrom(fd, (char *)&buffer_reply, REPLY_BUFFER_SIZE, 0,
               (struct sockaddr *)&srcaddr, &srcaddrlen);
  if (n < 0)
    perrorExit("recvfrom");

  printf("got reply from %s in %dms\n", argv[1], timediffMs(start_time));

  char mac[100] = {0};

  if (getMACAddress(argv[1], mac) != -1)
    printf("MAC-address for %s: %s\n", argv[1], mac);

  close(fd);
  return EXIT_SUCCESS;
}

int getMACAddress(const char *host, char *mac) {
  char line[200];
  char ip[100];

  /* Open the PROCps kernel table. */
  FILE *fp;
  if ((fp = fopen(_PATH_PROCNET_ARP, "r")) == NULL) {
    perror(_PATH_PROCNET_ARP);
    return -1;
  }
  if (fgets(line, sizeof(line), fp) != (char *)NULL) {
    for (; fgets(line, sizeof(line), fp);) {
      sscanf(line, "%s 0x%*x 0x%*x %99s\n", ip, mac);
      if (strcmp(ip, host) == 0)
        return 0;
    }
  }
  strcpy(mac, "(unknown)");
  return 0;
}

void perrorExit(const char *m) {
  perror(m);
  exit(EXIT_FAILURE);
}

int timediffMs(struct timeval from) {
  struct timeval now, result;

  gettimeofday(&now, NULL);
  timeval_subtract(&result, &now, &from);
  return result.tv_sec * 1000 + result.tv_usec / 1000;
}

/*
From https://github.com/guillemj/inetutils/blob/master/libicmp/icmp_cksum.c
*/
unsigned short icmp_cksum(unsigned char *addr, int len) {
  int sum = 0;
  unsigned short answer = 0;
  unsigned short *wp;

  for (wp = (unsigned short *)addr; len > 1; wp++, len -= 2)
    sum += *wp;

  /* Take in an odd byte if present */
  if (len == 1) {
    *(unsigned char *)&answer = *(unsigned char *)wp;
    sum += answer;
  }

  sum = (sum >> 16) + (sum & 0xffff); /* add high 16 to low 16 */
  sum += (sum >> 16);                 /* add carry */
  answer = ~sum;                      /* truncate to 16 bits */
  return answer;
}

/*
From https://www.gnu.org/software/libc/manual/html_node/Calculating-Elapsed-Time.html
*/

/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */
int timeval_subtract(struct timeval *result, struct timeval *x,
                     struct timeval *y) {
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

