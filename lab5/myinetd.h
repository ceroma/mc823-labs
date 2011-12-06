#include "sv_utils.h"

#define MAX_NAME_SIZE 20
#define MAX_PATH_SIZE 100
#define MAX_LINE_SIZE 200

#define SPACE_DELIMS " \t\n"

#define MYINETD_CONF_PATH "./myinetd.conf"

/**
 * Myinetd service type.
 */
typedef struct {
    int port;
    int sockfd;
    socktype_t protocol;
    char name[MAX_NAME_SIZE];
    char path[MAX_PATH_SIZE];
} service_t;

/**
 * List of services.
 */
typedef struct {
    int N;
    service_t * service;
} services_t;

/**
 * Reads the current line of myinetd.conf and returns a related service_t.
 */
service_t read_service(char * line);

/**
 * Reads and stores all services in myinetd.conf.
 */
services_t read_config();
