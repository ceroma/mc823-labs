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
#include "daemon.h"

#define MAXDATASIZE 100  /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    time_t now;
    socklen_t addr_len;
    char daytime[MAXDATASIZE];
    struct sockaddr their_addr;

    /* Retrieve client's address: */
    addr_len = sizeof(struct sockaddr);
    if (getpeername(fileno(stdin), &their_addr, &addr_len) == -1) {
        perror("getpeername");
        exit(1);
    }

    /* Log connection: */
    log_conn("udp_daytime", (struct sockaddr_in *)&their_addr);

    /* Prepare daytime information: */
    time(&now);
    sprintf(daytime, "%s", ctime(&now));

    /* Send daytime to client: */
    if (sendto(fileno(stdout), daytime, strlen(daytime), 0,
        (struct sockaddr *)&their_addr, addr_len) == -1) {
        perror("sendto");
        exit(1);
    }

    return 0;
}
