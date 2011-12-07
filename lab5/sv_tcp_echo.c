/*
** sv_tcp_echo.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXDATASIZE 256  /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    char recvline[MAXDATASIZE];

    /* Main connection loop - echo received data: */
    while (fgets(recvline, MAXDATASIZE, stdin)) {
        if (fflush(stdin) == EOF) {
            perror("fflush");
            exit(1);
        }
        if (puts(recvline) == EOF) {
            fprintf(stderr, "fputs: error writing to socket\n");
            exit(1);
        }
        if (fflush(stdout) == EOF) {
            perror("fflush");
            exit(1);
        }
    }

    return 0;
}
