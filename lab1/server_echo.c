/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define MYPORT      3490 /* the port users will be connecting to */
#define BACKLOG     10   /* how many pending connections queue will hold */
#define MAXDATASIZE 256  /* max number of bytes we can get at once */

main()
{
    int sockfd, new_fd;  /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr;    /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    char recvline[MAXDATASIZE];
    int num_rcvd, lines_rcvd;
    int sin_size, numbytes, yes;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
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

        /* Maintain connection in a new thread: */
        if (!fork()) {
            /* Main connection loop - echo received data: */
            num_rcvd = lines_rcvd = 0;
            while ((numbytes = recv(new_fd, recvline, MAXDATASIZE, 0)) > 0) {
                lines_rcvd++;
                num_rcvd += numbytes;
                if (send(new_fd, recvline, numbytes, 0) == -1) {
                    perror("send");
                    exit(1);
                }
            }

            if (numbytes == -1) {
                perror("recv");
                exit(1);
            }

            close(new_fd);

            /* Print statistics: */
            fprintf(stderr, "Number of lines received: %d\n", lines_rcvd);
            fprintf(stderr, "Number of characters received: %d\n", num_rcvd);
            exit(0);
        }
        close(new_fd);  /* parent doesn't need this */

        /* Clean up all child processes: */
        while (waitpid(-1, NULL, WNOHANG) > 0);
    }
}
