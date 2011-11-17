/*
** sv_tcp_daytime.c -- a stream socket daytime server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "sv_utils.h"

#define MYPORT      7332 /* the port users will be connecting to */
#define MAXDATASIZE 100  /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    time_t now;
    int sockfd, new_fd;  /* listen on sock_fd, new connection on new_fd */
    char daytime[MAXDATASIZE];

    /* Accept new connection: */
    sockfd = tcp_socket(MYPORT);
    new_fd = tcp_accept(sockfd);

    /* Prepare daytime information: */
    time(&now);
    sprintf(daytime, "%s", ctime(&now));

    /* Send daytime to client: */
    if (write(new_fd, daytime, strlen(daytime)) == -1) {
        perror("write");
        exit(1);
    }

    /* Close connection: */
    if (close(new_fd) == -1) {
        perror("close");
        exit(1);
    }
    if (close(sockfd) == -1) {
        perror("close");
        exit(1);
    }

    return 0;
}
