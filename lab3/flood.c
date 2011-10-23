/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUFLEN  256  // max number of bytes we can get at once
#define SERVERPORT 4950 // the port users will be connecting to

int main(int argc, char *argv[])
{
    clock_t t1, t2;
    double time_diff;
    int sockfd;
    int size_sent, max_size;
    int num_sent, packets_sent;
    char buf[MAXBUFLEN];
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;

    if (argc != 2) {
        fprintf(stderr, "usage: talker hostname\n");
        exit(1);
    }

    if ((he = gethostbyname(argv[1])) == NULL) { // get the host info
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;         // host byte order
    their_addr.sin_port = htons(SERVERPORT); // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8); // zero the rest of the struct

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))
        == -1) {
        perror("connect");
        exit(1);
    }

    /* Start counter: */
    if ((t1 = times(NULL)) == (clock_t) -1) {
        perror("times");
        exit(1);
    }

    /* Read stdin until EOF: */
    num_sent = packets_sent = max_size = 0;
    while (fgets(buf, MAXBUFLEN, stdin)) {
        /* Send data to server: */
        if (send(sockfd, buf, strlen(buf), 0) == -1) {
            perror("send");
            exit(1);
        }

        /* Statistics - send: */
        packets_sent++;
        size_sent = strlen(buf);
        num_sent += size_sent;
        max_size = (size_sent > max_size) ? size_sent : max_size;
    }

    /* Send end-of-transmission signal to server: */
    sleep(1);
    if (send(sockfd, NULL, 0, 0) == -1) {
        perror("send");
        exit(1);
    }

    /* Unset peer name: */
    their_addr.sin_family = AF_UNSPEC;
    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))
        == -1) {
        perror("connect");
        exit(1);
    }
    close(sockfd);

    /* Stop counter: */
    if ((t2 = times(NULL)) == (clock_t) -1) {
        perror("times");
        exit(1);
    }

    /* Print statistics: */
    time_diff = ((double) t2 - (double) t1) / sysconf(_SC_CLK_TCK);
    fprintf(stderr, "Number of packets sent: %d\n", packets_sent);
    fprintf(stderr, "Size of longest packet: %d\n", max_size);
    fprintf(stderr, "Number of bytes sent: %d\n", num_sent);
    fprintf(stderr, "Time: %.1lfs\n", time_diff);

    return 0;
}
