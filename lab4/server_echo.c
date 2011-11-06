/*
** server.c -- a stream socket server demo
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define MYPORT      3490 /* the port users will be connecting to */
#define BACKLOG     10   /* how many pending connections queue will hold */
#define MAXDATASIZE 256  /* max number of bytes we can get at once */

/**
 * Daemonizes the server.
 */
void daemon_init() {
    int i = 0;
    pid_t pid;

    /* Terminate parent: */
    if ((pid = fork()) == -1) {
        perror("fork");
        exit(1);
    } else if (pid != 0) {
        exit(0);
    }

    /* Become session leader, detaching from terminal: */
    if (setsid() == -1) {
        perror("setsid");
        exit(1);
    }

    /* Ignore children termination: */
    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    /* Terminate first child: */
    if ((pid = fork()) == -1) {
        perror("fork");
        exit(1);
    } else if (pid != 0) {
        exit(0);
    }

    /* Change working directory: */
    if (chdir("/tmp") == -1) {
        perror("chdir");
        exit(1);
    }

    /* Clear file mode creation mask: */
    umask(0);

    /* Close all file descriptors: */
    do { close(i); } while (++i < getdtablesize());

    /* Tell syslog to include PID with each message: */
    openlog("server-echo", LOG_PID, 0);
}

/**
 * Logs message to a private file.
 */
void log_message(char *message) {
    FILE *fp;
    time_t now;
    char buf[64];

    /* Build log header: */
    time(&now);
    sprintf(buf, "%sserver-echo[%d]: ", ctime(&now), getpid());

    /* Open private log file: */
    if ((fp = fopen("SERVER.LOG", "a")) == 0) {
        exit(1);
    }

   /* Logs message: */
   fprintf(fp, "%s %s\n", buf, message);
   fclose(fp);
}

/**
 * Logs a new connection to the server.
 */
void log_conn(struct in_addr in) {
    //syslog(LOG_USER | LOG_INFO, "Got connection from: %s\n", inet_ntoa(in));
    char buf[64];
    sprintf(buf, "Got connection from: %s\n", inet_ntoa(in));
    log_message(buf);
}

/**
 * Logs connection's statistics.
 */
void log_stats(int num_rcvd, int lin_rcvd) {
    //syslog(LOG_USER | LOG_INFO, "Number of bytes received: %d\n", num_rcvd);
    //syslog(LOG_USER | LOG_INFO, "Number of lines received: %d\n", lin_rcvd);
    char buf[64];
    sprintf(buf, "Number of bytes received: %d\n", num_rcvd);
    log_message(buf);
    sprintf(buf, "Number of lines received: %d\n", lin_rcvd);
    log_message(buf);
}

/**
 * Logs an execution error.
 */
void log_error(char *fname) {
    //syslog(LOG_USER | LOG_ERR, "%s: %m\n", fname);
    char buf[64];
    sprintf(buf, "%s: %s\n", fname, strerror(errno));
    log_message(buf);
}

main()
{
    int sockfd, new_fd;  /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr;    /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    char recvline[MAXDATASIZE];
    int num_rcvd, lines_rcvd;
    int sin_size, yes;
    FILE *rsock, *wsock;

    daemon_init();

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        log_error("socket");
        exit(1);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        log_error("setsockopt");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         /* host byte order */
    my_addr.sin_port = htons(MYPORT);     /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* automatically fill with my IP */
    bzero(&(my_addr.sin_zero), 8);        /* zero the rest of the struct */

    if (-1 ==
        bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))) {
        log_error("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        log_error("listen");
        exit(1);
    }

    while (1) {  /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size))
            == -1) {
            log_error("accept");
            continue;
        }

        /* Associate a write stream with the socket file descriptor: */
        if ((wsock = fdopen(new_fd, "w")) == NULL) {
            log_error("fdopen");
            exit(1);
        }

        /* Associate a read stream with the socket file descriptor: */
        if ((rsock = fdopen(new_fd, "r")) == NULL) {
            log_error("fdopen");
            exit(1);
        }

        /* Maintain connection in a new thread: */
        if (!fork()) {
            /* Main connection loop - echo received data: */
            num_rcvd = lines_rcvd = 0;
            while (fgets(recvline, MAXDATASIZE, rsock)) {
                if (fflush(rsock) == EOF) {
                    log_error("fflush");
                    exit(1);
                }
                lines_rcvd++;
                num_rcvd += strlen(recvline);
                if (fputs(recvline, wsock) == EOF) {
                    log_error("fputs: error writing to socket");
                    exit(1);
                }
                if (fflush(wsock) == EOF) {
                    log_error("fflush");
                    exit(1);
                }
            }
            if (close(new_fd) == -1) {
                log_error("close");
                exit(1);
            }
            free(rsock);
            free(wsock);

            /* Print statistics: */
            log_conn(their_addr.sin_addr);
            log_stats(num_rcvd, lines_rcvd);
            exit(0);
        }
        if (close(new_fd) == -1) {  /* parent doesn't need this */
            log_error("close");
            exit(1);
        }

        /* Clean up all child processes: */
        while (waitpid(-1, NULL, WNOHANG) > 0);
    }
}
