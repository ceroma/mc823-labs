#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>
#include "myinetd.h"

services_t s;

/**
 * Reads the current line of myinetd.conf and returns a related service_t.
 */
service_t read_service(char * line) {
    service_t service;
    char * field;

    /* Initialize PID: */
    service.pid = 0;

    /* Read name: */
    field = strtok(line, SPACE_DELIMS);
    strcpy(service.name, field); 

    /* Read port: */
    field = strtok(NULL, SPACE_DELIMS);
    service.port = atoi(field);

    /* Read protocol: */
    strtok(NULL, SPACE_DELIMS);
    field = strtok(NULL, SPACE_DELIMS);
    if (!strcmp(field, "tcp")) {
        service.protocol = TCP;
    } else {
        service.protocol = UDP;
    }

    /* Read pathname: */
    strtok(NULL, SPACE_DELIMS);
    field = strtok(NULL, SPACE_DELIMS);
    strcpy(service.path, field); 

    /* Create socket: */
    service.sockfd = tcpudp_socket(service.port, service.protocol);

    return service;
}

/**
 * Reads and stores all services in myinetd.conf.
 */
services_t read_config() {
    FILE * conf;
    services_t s;
    char line[MAX_LINE_SIZE];

    /* Open config file: */
    conf = fopen(MYINETD_CONF_PATH, "r");

    /* Read services in config file: */
    s.N = 0;
    s.service = NULL;
    while (fgets(line, MAX_LINE_SIZE, conf)) {
        s.N++;
        s.service = realloc(s.service, s.N * sizeof(service_t));
        s.service[s.N - 1] = read_service(line);
    }
    
    fclose(conf);
    return s;
}

/**
 * Prints details of the services in s.
 */
void print_services(services_t s) {
    int i;

    printf("Services:\n");
    printf("i: Name\t\tPort\tSockFd\tType\tPath\n");
    for (i = 0; i < s.N; i++) {
        printf(
            "%d: %s\t%d\t%d\t%s\t%s\n",
            i,
            s.service[i].name,
            s.service[i].port,
            s.service[i].sockfd,
            s.service[i].protocol == TCP ? "tcp" : "udp",
            s.service[i].path
        );
    }
}

/**
 * Executes a given service.
 */
void execute_service(service_t service, int new_fd) {
    int i = 0;
    char * const args[] = {'\0'};

    /* Close all descriptors, except the connected socket: */
    do {
       if (i != new_fd) close(i);
    } while (++i < getdtablesize());

    /* Duplicate connected socket over stdin, stdout and stderr: */
    for (i = 0; i < 3; i++) {
        if (dup2(new_fd, i) == -1) {
            perror("dup2");
            exit(1);
        }
    }
    close(new_fd);

    /* Execute service: */
    if (execv(service.path, args) == -1) {
        perror("execv");
        exit(1);
    }
    exit(0);
}

/**
 * Handles the death of the services. If the dead child was an UDP service,
 * unblocks it's socket descriptor.
 */
void signal_handler(int sig) {
    int i;
    pid_t pid;

    /* Only handle dead children: */
    if (sig != SIGCHLD) {
        return;
    }

    /* Recover child's PID: */
    pid = wait(NULL);

    /* Restores UDP service: */
    for (i = 0; i < s.N; i++) {
        if (s.service[i].pid == pid) {
            s.service[i].pid = 0;
            s.service[i].sockfd = udp_socket(s.service[i].port);
        }
    }
}

int main(int argc, char *argv[]) {
    fd_set readfds;
    int i, maxfd, new_fd;

    /* Reads myinetd.conf: */
    s = read_config();

    /* Set up SIGCHLD handler: */
    signal(SIGCHLD, signal_handler);

    /* Find greater socket descriptor: */
    maxfd = 0;
    for (i = 0; i < s.N; i++) {
        maxfd = (maxfd < s.service[i].sockfd) ? s.service[i].sockfd : maxfd;
    }

    /* Execute connection requests: */
    while (1) {
        /* Specify file descriptors to be checked for being ready: */
        FD_ZERO(&readfds);
        for (i = 0; i < s.N; i++) {
            if (!s.service[i].pid) {
                FD_SET(s.service[i].sockfd, &readfds);
            }
        }

        /* Wait until one of the inputs become ready for read: */
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) == -1) {
            if (errno == EINTR) continue;

            perror("select");
            exit(1);
        }

        /* Find out which services were called and execute them: */
        for (i = 0; i < s.N; i++) {
            if (FD_ISSET(s.service[i].sockfd, &readfds)) {
                if (s.service[i].protocol == TCP) {
                    new_fd = tcp_accept(s.service[i].sockfd);
                    if (!fork()) { execute_service(s.service[i], new_fd); }
                    close(new_fd);
                } else {
                    FD_CLR(s.service[i].sockfd, &readfds);
                    udp_accept(s.service[i].sockfd);
                    if (!(s.service[i].pid = fork())) {
                        execute_service(s.service[i], s.service[i].sockfd);
                    }
                    close(s.service[i].sockfd);
                }
            }
        }

        /* Clean up all child processes: */
        while (waitpid(-1, NULL, WNOHANG) > 0);
    }

    /* Free everything: */
    for (i = 0; i < s.N; i++) {
        close(s.service[i].sockfd);
    }
    free(s.service);

    return 0;
}
