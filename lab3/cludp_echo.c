/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUFLEN  100  // max number of bytes we can get at once
#define SERVERPORT 4950 // the port users will be connecting to

int main(int argc, char *argv[])
{
    int sockfd;
    clock_t t1, t2;
    double time_diff;
    char buf[MAXBUFLEN];
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    socklen_t addr_len;
    int numbytes;

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
    addr_len = sizeof(struct sockaddr);

    /* Start counter: */
    if ((t1 = times(NULL)) == (clock_t) -1) {
        perror("times");
        exit(1);
    }

    /* Read stdin until EOF: */
    while (fgets(buf, MAXBUFLEN, stdin)) {
        /* Send data to server: */
        if ((numbytes = sendto(sockfd, buf, strlen(buf), 0,
            (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
            perror("sendto");
            exit(1);
        }

        /* Echo data received from server: */
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';
        fputs(buf, stdout);
    }

    /* Send end-of-transmission signal to server: */
    if ((numbytes = sendto(sockfd, NULL, 0, 0,
        (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
        perror("sendto");
        exit(1);
    }

    /* Stop counter: */
    if ((t2 = times(NULL)) == (clock_t) -1) {
        perror("times");
        exit(1);
    }

    /* Print statistics: */
    time_diff = ((double) t2 - (double) t1) / sysconf(_SC_CLK_TCK);
    fprintf(stderr, "Number of bytes sent: %d\n", numbytes);
    fprintf(stderr, "Time: %.1lfs\n", time_diff);

    close(sockfd);

    return 0;
}
