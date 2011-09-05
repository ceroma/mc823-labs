/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/times.h>

#define PORT        3490 /* the port client will be connecting to */
#define MAXDATASIZE 256  /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    clock_t t1, t2;
    double time_diff;
    int sockfd, numbytes;  
    int num_sent, num_rcvd;
    int size_sent, max_size;
    int lines_sent, lines_rcvd;
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */

    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    if ((he = gethostbyname(argv[1])) == NULL) {  /* get the host info */
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;         /* host byte order */
    their_addr.sin_port = htons(PORT);       /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);        /* zero the rest of the struct */

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
    num_sent = num_rcvd = lines_sent = lines_rcvd = max_size = 0;
    while (fgets(buf, MAXDATASIZE, stdin)) {
        /* Send data to server: */
        if (send(sockfd, buf, strlen(buf), 0) == -1) {
            perror("send");
            exit(1);
        }

        /* Statistics - send: */
        lines_sent++;
        size_sent = strlen(buf);
        num_sent += size_sent;
        max_size = (size_sent > max_size) ? size_sent : max_size;

        /* Echo data received from server: */
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';
        fputs(buf, stdout);

        /* Statistics - recv: */
        lines_rcvd++;
        num_rcvd += numbytes;
    }
    close(sockfd);

    /* Stop counter: */
    if ((t2 = times(NULL)) == (clock_t) -1) {
        perror("times");
        exit(1);
    }

    /* Print statistics: */
    time_diff = ((double) t2 - (double) t1) / sysconf(_SC_CLK_TCK);
    fprintf(stderr, "Number of lines sent: %d\n", lines_sent);
    fprintf(stderr, "Size of longest line: %d\n", max_size);
    fprintf(stderr, "Number of characters sent: %d\n", num_sent);
    fprintf(stderr, "Number of lines received: %d\n", lines_rcvd);
    fprintf(stderr, "Number of characters received: %d\n", num_rcvd);
    fprintf(stderr, "Time: %.1lfs", time_diff);

    return 0;
}
