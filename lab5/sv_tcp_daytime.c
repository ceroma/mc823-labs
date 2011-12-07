/*
** sv_tcp_daytime.c -- a stream socket daytime server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[])
{
    time_t now;

    /* Prepare daytime information: */
    time(&now);
    printf("%s", ctime(&now));

    return 0;
}
