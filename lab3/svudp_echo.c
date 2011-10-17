/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXBUFLEN 256
#define MYPORT    4950 // the port users will be connecting to

/**
 * If SIGALRM was detected, server has been waiting a packet for too long and
 * should terminate the current connection.
 */
void catch_alarm(int sig) {
    fprintf(stderr, "Connection timed out!\n");
}

int main(void)
{
    struct sigaction action;
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t addr_len;
    int sockfd, numbytes;
    int num_sent, num_rcvd;
    int packets_sent, packets_rcvd;
    char buf[MAXBUFLEN];

    /* Establish a handler for SIGALRM signals: */
    action.sa_flags = 0;
    action.sa_handler = catch_alarm;
    if (sigemptyset(&(action.sa_mask)) == -1) {
        perror("sigemptyset");
        exit(1);
    }
    if (sigaction(SIGALRM, &action, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct
    addr_len = sizeof(struct sockaddr);

    while (1) {
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        if (bind(sockfd,
            (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
            perror("bind");
            exit(1);
        }

        /* Receive first transmission: */
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        /* Set peer name: */
        if (connect(sockfd,
            (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
            perror("connect");
            exit(1);
        }

        /* Main connection loop - echo data until end-of-transmission: */
        num_sent = num_rcvd = packets_sent = packets_rcvd = 0;
        do {
            /* Check recv error: */
            if (numbytes == -1) {
                /* Interrupted by a signal: */
                if (errno == EINTR) { break; }

                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';

            /* Statistics - recv: */
            packets_rcvd++;
            num_rcvd += numbytes;

            /* Echo data to client: */
            if (send(sockfd, buf, strlen(buf), 0) == -1) {
                perror("send");
                exit(1);
            }

            /* Statistics - send: */
            packets_sent++;
            num_sent += strlen(buf);

            /* Set timeout alarm: */
            alarm(2);

            /* Receive data from client: */
        } while ((numbytes = recv(sockfd, buf, MAXBUFLEN-1, 0)) != 0);

        /* Turn alarm off: */
        alarm(0);

        /* Unset peer name: */
        their_addr.sin_family = AF_UNSPEC;
        if (connect(sockfd,
            (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
            perror("connect");
            exit(1);
        }
        close(sockfd);

        /* Print statistics: */
        fprintf(stderr, "Packets from: %s\n", inet_ntoa(their_addr.sin_addr));
        fprintf(stderr, "Number of packets sent: %d\n", packets_sent);
        fprintf(stderr, "Number of bytes sent: %d\n", num_sent);
        fprintf(stderr, "Number of packets received: %d\n", packets_rcvd);
        fprintf(stderr, "Number of bytes received: %d\n", num_rcvd);
    }

    return 0;
}
