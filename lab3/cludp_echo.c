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

/**
 * If SIGALRM was detected, server has been waiting a packet for too long and
 * should terminate the current connection.
 */
void catch_alarm(int sig) {
    fprintf(stderr, "Connection timed out!\n");
}

int main(int argc, char *argv[])
{
    clock_t t1, t2;
    double time_diff;
    int sockfd, numbytes;
    int num_sent, num_rcvd;
    int size_sent, max_size;
    int packets_sent, packets_rcvd;
    char buf[MAXBUFLEN];
    struct sigaction action;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;

    if (argc != 2) {
        fprintf(stderr, "usage: talker hostname\n");
        exit(1);
    }

    /* Establish a handler for SIGALRM signals: */
    action.sa_flags = 0;
    action.sa_handler = catch_alarm;
    if (sigemptyset(&(action.sa_mask)) == -1) {
        perror("sigemptyset");
        exit(1);
    }
    if (sigaction(SIGALRM, &action, NULL) == -1) {
        perror("sigaction");
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
    num_sent = num_rcvd = packets_sent = packets_rcvd = max_size = 0;
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

        /* Echo data received from server: */
        alarm(2);
        if ((numbytes = recv(sockfd, buf, MAXBUFLEN-1, 0)) == -1) {
            /* Interrupted by a signal: */
            if (errno == EINTR) { break; }

            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';
        fputs(buf, stdout);

        /* Statistics - recv: */
        packets_rcvd++;
        num_rcvd += numbytes;
    }

    /* Send end-of-transmission signal to server: */
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
    fprintf(stderr, "Number of packets received: %d\n", packets_rcvd);
    fprintf(stderr, "Number of bytes received: %d\n", num_rcvd);
    fprintf(stderr, "Time: %.1lfs\n", time_diff);

    return 0;
}
