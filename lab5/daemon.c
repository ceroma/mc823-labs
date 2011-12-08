#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
    openlog("myinetd", LOG_PID, 0);
}

/**
 * Logs message to a private file.
 */
void log_message(char *caller, char *message) {
    FILE *fp;
    time_t now;
    char buf[64];

    /* Build log header: */
    time(&now);
    sprintf(buf, "%s%s[%d]: ", ctime(&now), caller, getpid());

    /* Open private log file: */
    if ((fp = fopen("SERVER.LOG", "a")) == 0) {
        exit(1);
    }

   /* Log message: */
   fprintf(fp, "%s %s\n", buf, message);
   fflush(fp);
   fclose(fp);
}

/**
 * Logs a new connection to the server.
 */
void log_conn(char *caller, struct sockaddr_in in) {
    char buf[64];
    int port = ntohs(in.sin_port);
    sprintf(buf, "Client: %s - %d\n", inet_ntoa(in.sin_addr), port);
    log_message(caller, buf);
}

/**
 * Logs an execution error.
 */
void log_error(char *caller, char *fname) {
    char buf[64];
    sprintf(buf, "%s: %s\n", fname, strerror(errno));
    log_message(caller, buf);
}
