#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sv_utils.h"

#define BACKLOG     10   /* how many pending connections queue will hold */

/**
 * Returns a TCP socket listening to connections from the given port or an UDP
 * datagram.
 */
int tcpudp_socket(int port, socktype_t type) {
    int sockfd, yes, socktype;
    struct sockaddr_in my_addr;    /* my address information */

    /* Create socket: */
    socktype = (type == TCP) ? SOCK_STREAM : SOCK_DGRAM;
    if ((sockfd = socket(AF_INET, socktype, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* TCP - allow multiple connections: */
    if (type == TCP) {
        if (-1 ==
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
            perror("setsockopt");
            exit(1);
        }
    }

    my_addr.sin_family = AF_INET;         /* host byte order */
    my_addr.sin_port = htons(port);       /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* automatically fill with my IP */
    bzero(&(my_addr.sin_zero), 8);        /* zero the rest of the struct */

    /* Assign address to socket: */
    if (-1 ==
        bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))) {
        perror("bind");
        exit(1);
    }

    /* TCP - mark as listening to incoming connections: */
    if (type == TCP) {
        if (listen(sockfd, BACKLOG) == -1) {
            perror("listen");
            exit(1);
        }
    }

    return sockfd;
}

/**
 * Returns an UDP datagram.
 */
int udp_socket(int port) {
    return tcpudp_socket(port, UDP);
}

/**
 * Returns a TCP socket listening to connections from the given port.
 */
int tcp_socket(int port) {
    return tcpudp_socket(port, TCP);
}

/**
 * Waits for a new connection request and logs the connector's address. Returns
 * a file descriptor referring to the accepted socket, if TCP.
 */
int tcpudp_accept(int sockfd, socktype_t type) {
    socklen_t sin_size;
    int new_fd = sockfd;
    struct sockaddr_in their_addr; /* connector's address information */

    sin_size = sizeof(struct sockaddr_in);

    if (type == TCP) {
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size))
            == -1) {
            perror("accept");
            exit(1);
        }
    } else {
        if (recvfrom(sockfd, NULL, 0, MSG_PEEK,
            (struct sockaddr *)&their_addr, &sin_size) == -1) {
            perror("recvfrom");
            exit(1);
        }
    }

    printf(
        "server: got connection from %s\n",
        inet_ntoa(their_addr.sin_addr)
    );

    return new_fd;
}

/**
 * Waits for a new connection request and returns a file descriptor referring
 * to the accepted socket.
 */
int tcp_accept(int sockfd) {
    return tcpudp_accept(sockfd, TCP);
}

/**
 * Waits for an UDP datagram and logs the connector's address.
 */
int udp_accept(int sockfd) {
    return tcpudp_accept(sockfd, UDP);
}
