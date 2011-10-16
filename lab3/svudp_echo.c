/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXBUFLEN 256
#define MYPORT    4950 // the port users will be connecting to

int main(void)
{
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t addr_len;
    int sockfd, numbytes;
    int num_sent, num_rcvd;
    int packets_sent, packets_rcvd;
    char buf[MAXBUFLEN];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct
    addr_len = sizeof(struct sockaddr);

    if (-1 ==
        bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))) {
        perror("bind");
        exit(1);
    }

    /* Main connection loop - echo received data until end-of-transmission: */
    num_sent = num_rcvd = packets_sent = packets_rcvd = 0;
    while ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
        (struct sockaddr *)&their_addr, &addr_len)) != 0) {
        if (numbytes == -1) {
            perror("recvfrom");
            exit(1);
        }

        /* Statistics - recv: */
        packets_rcvd++;
        num_rcvd += numbytes;

        /* Echo data to client: */
        if ((numbytes = sendto(sockfd, buf, numbytes, 0,
            (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
            perror("sendto");
            exit(1);
        }

        /* Statistics - send: */
        packets_sent++;
        num_sent += numbytes;
    }
    close(sockfd);

    /* Print statistics: */
    fprintf(stderr, "Got packets from %s\n", inet_ntoa(their_addr.sin_addr));
    fprintf(stderr, "Number of packets sent: %d\n", packets_sent);
    fprintf(stderr, "Number of bytes sent: %d\n", num_sent);
    fprintf(stderr, "Number of packets received: %d\n", packets_rcvd);
    fprintf(stderr, "Number of bytes received: %d\n", num_rcvd);

    return 0;
}
