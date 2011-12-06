/*
** sv_udp_daytime.c -- a UDP daytime server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sv_utils.h"

#define MYPORT      7333 /* the port users will be connecting to */
#define MAXDATASIZE 100  /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    time_t now;
    int sockfd;
    socklen_t addr_len;
    char daytime[MAXDATASIZE];
    struct sockaddr their_addr;

    /* Create datagram: */
    sockfd = udp_socket(MYPORT);

    /* Receive client's address: */
    addr_len = sizeof(struct sockaddr);
    if (recvfrom(sockfd, daytime, MAXDATASIZE-1, 0,
        (struct sockaddr *)&their_addr, &addr_len) == -1) {
        perror("recvfrom");
        exit(1);
    }

    /* Prepare daytime information: */
    time(&now);
    sprintf(daytime, "%s", ctime(&now));

    /* Send daytime to client: */
    if (sendto(sockfd, daytime, strlen(daytime), 0,
        (struct sockaddr *)&their_addr, addr_len) == -1) {
        perror("sendto");
        exit(1);
    }

    /* Close datagram: */
    if (close(sockfd) == -1) {
        perror("close");
        exit(1);
    }

    return 0;
}
