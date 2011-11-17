/*
** sv_tcp_echo.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sv_utils.h"

#define MYPORT      7331 /* the port users will be connecting to */
#define MAXDATASIZE 256  /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    int sockfd, new_fd;  /* listen on sock_fd, new connection on new_fd */
    char recvline[MAXDATASIZE];
    int num_rcvd, lines_rcvd;
    FILE *rsock, *wsock;

    /* Accept new connection: */
    sockfd = tcp_socket(MYPORT);
    new_fd = tcp_accept(sockfd);

    /* Associate a write stream with the socket file descriptor: */
    if ((wsock = fdopen(new_fd, "w")) == NULL) {
        perror("fdopen");
        exit(1);
    }

    /* Associate a read stream with the socket file descriptor: */
    if ((rsock = fdopen(new_fd, "r")) == NULL) {
        perror("fdopen");
        exit(1);
    }

    /* Main connection loop - echo received data: */
    num_rcvd = lines_rcvd = 0;
    while (fgets(recvline, MAXDATASIZE, rsock)) {
        if (fflush(rsock) == EOF) {
            perror("fflush");
            exit(1);
        }
        lines_rcvd++;
        num_rcvd += strlen(recvline);
        if (fputs(recvline, wsock) == EOF) {
            fprintf(stderr, "fputs: error writing to socket\n");
            exit(1);
        }
        if (fflush(wsock) == EOF) {
            perror("fflush");
            exit(1);
        }
    }
    if (close(new_fd) == -1) {
        perror("close");
        exit(1);
    }
    if (close(sockfd) == -1) {
        perror("close");
        exit(1);
    }
    free(rsock);
    free(wsock);

    /* Print statistics: */
    fprintf(stderr, "Number of lines received: %d\n", lines_rcvd);
    fprintf(stderr, "Number of characters received: %d\n", num_rcvd);

    return 0;
}
