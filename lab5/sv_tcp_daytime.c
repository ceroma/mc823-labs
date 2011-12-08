/*
** sv_tcp_daytime.c -- a stream socket daytime server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "daemon.h"

int main(int argc, char *argv[])
{
    time_t now;
    socklen_t addr_len;
    struct sockaddr their_addr;

    /* Retrieve client's address: */
    addr_len = sizeof(struct sockaddr);
    if (getpeername(fileno(stdin), &their_addr, &addr_len) == -1) {
        perror("getpeername");
        exit(1);
    }

    /* Log connection: */
    log_conn("tcp_daytime", (struct sockaddr_in *)&their_addr);

    /* Prepare daytime information: */
    time(&now);
    printf("%s", ctime(&now));

    return 0;
}
