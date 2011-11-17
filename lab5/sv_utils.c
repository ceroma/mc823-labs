#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG     10   /* how many pending connections queue will hold */

/**
 * Returns a TCP socket listening to connections from the given port.
 */
int tcp_socket(int port) {
    int sockfd, yes;
    struct sockaddr_in my_addr;    /* my address information */

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         /* host byte order */
    my_addr.sin_port = htons(port);       /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* automatically fill with my IP */
    bzero(&(my_addr.sin_zero), 8);        /* zero the rest of the struct */

    if (-1 ==
        bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    return sockfd;
}

/**
 * Waits for a new connection request and returns a file descriptor referring
 * to the accepted socket.
 */
int tcp_accept(int sockfd) {
    int new_fd;
    socklen_t sin_size;
    struct sockaddr_in their_addr; /* connector's address information */

    sin_size = sizeof(struct sockaddr_in);
    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size))
        == -1) {
        perror("accept");
        exit(1);
    }

    printf(
        "server: got connection from %s\n",
        inet_ntoa(their_addr.sin_addr)
    );

    return new_fd;
}
