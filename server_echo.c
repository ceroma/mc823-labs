/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define MYPORT      3490 /* the port users will be connecting to */
#define BACKLOG     10   /* how many pending connections queue will hold */
#define MAXDATASIZE 100  /* max number of bytes we can get at once */

main()
{
    int sockfd, new_fd;  /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr;    /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    char buf[MAXDATASIZE];
    int sin_size, numbytes;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         /* host byte order */
    my_addr.sin_port = htons(MYPORT);     /* short, network byte order */
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

    while (1) {  /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size))
            == -1) {
            perror("accept");
            continue;
        }

        printf(
            "server: got connection from %s\n",
            inet_ntoa(their_addr.sin_addr)
        );

        /* Main connection loop - echo received data: */
        while ((numbytes = recv(new_fd, buf, MAXDATASIZE, 0)) > 0) {
            if (send(new_fd, buf, numbytes, 0) == -1) {
                perror("send");
                exit(1);
            }
        }

        if (numbytes == -1) {
            perror("recv");
            exit(1);
        }

        close(new_fd);  
    }
}
