/*
** cl_tcp_echo.c -- a stream socket client demo
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
#include <sys/wait.h>

#define PORT        7331 /* the port client will be connecting to */
#define MAXDATASIZE 256  /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    fd_set readfds;
    clock_t t1, t2;
    double time_diff;
    int sockfd, nfds;
    int num_sent, num_rcvd;
    int size_sent, max_size;
    int lines_sent, lines_rcvd;
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */
    FILE *rsock, *wsock;

    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    /* Guarantee standard I/O is line buffered: */
    if (setvbuf(stdin, (char *) NULL, _IOLBF, 0)) {
        perror("setvbuf");
        exit(1);
    }
    if (setvbuf(stdout, (char *) NULL, _IOLBF, 0)) {
        perror("setvbuf");
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

    /* Associate a write stream with the socket file descriptor: */
    if ((wsock = fdopen(sockfd, "w")) == NULL) {
        perror("fdopen");
        exit(1);
    }

    /* Make write stream line buffered: */
    if (setvbuf(wsock, (char *) NULL, _IOLBF, 0)) {
        perror("setvbuf");
        exit(1);
    }

    /* Associate a read stream with the socket file descriptor: */
    if ((rsock = fdopen(sockfd, "r")) == NULL) {
        perror("fdopen");
        exit(1);
    }

    /* Make read stream line buffered: */
    if (setvbuf(rsock, (char *) NULL, _IOLBF, 0)) {
        perror("setvbuf");
        exit(1);
    }

    /* Initialize I/O multiplexing: */
    nfds = sockfd + 1;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    FD_SET(fileno(stdin), &readfds);

    /* Start counter: */
    if ((t1 = times(NULL)) == (clock_t) -1) {
        perror("times");
        exit(1);
    }

    /* Read stdin until EOF: */
    num_sent = num_rcvd = lines_sent = lines_rcvd = max_size = 0;
    while (!feof(stdin) || !feof(rsock)) {
        /* Specify file descriptors to be checked for being ready: */
        FD_ZERO(&readfds);
        if (!feof(rsock)) FD_SET(sockfd, &readfds);
        if (!feof(stdin)) FD_SET(fileno(stdin), &readfds);

        /* Wait until one of the inputs become ready for read: */
        if (select(nfds, &readfds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        /* Standard input is ready: */
        if (FD_ISSET(fileno(stdin), &readfds)) {
            if (!fgets(buf, MAXDATASIZE, stdin)) {
                /* Disallow further transmissions: */
                if (shutdown(sockfd, SHUT_WR) == -1) {
                    perror("shutdown");
                    exit(1);
                }
                continue;
            }

            /* Send data to server: */
            if (fputs(buf, wsock) == EOF) {
                fprintf(stderr, "fputs: error writing to socket\n");
                exit(1);
            }

            /* Statistics - send: */
            lines_sent++;
            size_sent = strlen(buf);
            num_sent += size_sent;
            max_size = (size_sent > max_size) ? size_sent : max_size;
        }

        /* Socket is ready: */
        if (FD_ISSET(sockfd, &readfds)) {
            if (!fgets(buf, MAXDATASIZE, rsock)) {
                continue;
            }

            /* Echo data received from server: */
            fputs(buf, stdout);

            /* Statistics - recv: */
            lines_rcvd++;
            num_rcvd += strlen(buf);
        }
    }

    if (close(sockfd) == -1) {
        perror("close");
        exit(1);
    }
    free(wsock);
    free(rsock);

    /* Stop counter: */
    if ((t2 = times(NULL)) == (clock_t) -1) {
        perror("times");
        exit(1);
    }

    /* Print statistics: */
    time_diff = ((double) t2 - (double) t1) / sysconf(_SC_CLK_TCK);
    fprintf(stderr, "Time: %.1lfs\n", time_diff);
    fprintf(stderr, "Number of lines sent: %d\n", lines_sent);
    fprintf(stderr, "Size of longest line: %d\n", max_size);
    fprintf(stderr, "Number of characters sent: %d\n", num_sent);
    fprintf(stderr, "Number of lines received: %d\n", lines_rcvd);
    fprintf(stderr, "Number of characters received: %d\n", num_rcvd);

    return 0;
}
