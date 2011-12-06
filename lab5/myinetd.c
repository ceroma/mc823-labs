#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "myinetd.h"

/**
 * Reads the current line of myinetd.conf and returns a related service_t.
 */
service_t read_service(char * line) {
    service_t service;
    char * field;

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

int main(int argc, char *argv[]) {
    int i;
    services_t s;

    /* Reads myinetd.conf: */
    s = read_config();

    /* Verify read data: */
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

    /* Free everything: */
    for (i = 0; i < s.N; i++) {
        close(s.service[i].sockfd);
    }
    free(s.service);

    return 0;
}
