#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * Returns a TCP socket connected to the given hostname and port.
 */
int cl_tcp_socket(char *hostname, int port) {
    int sockfd;
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */

    if ((he = gethostbyname(hostname)) == NULL) {  /* get the host info */
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;         /* host byte order */
    their_addr.sin_port = htons(port);       /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);        /* zero the rest of the struct */

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))
        == -1) {
        perror("connect");
        exit(1);
    }

    return sockfd;
}
