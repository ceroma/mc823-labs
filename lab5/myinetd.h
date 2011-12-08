#include "sv_utils.h"

#define MAX_NAME_SIZE 20
#define MAX_PATH_SIZE 100
#define MAX_LINE_SIZE 200

#define SPACE_DELIMS " \t\n"

#define MYINETD_CONF_PATH "/home/ec2006/ra059582/mc823/git/mc823-labs/lab5/myinetd.conf"

/**
 * Myinetd service type.
 */
typedef struct {
    int pid;
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

/**
 * Prints details of the services in s.
 */
void print_services(services_t s);

/**
 * Executes a given service.
 */
void execute_service(service_t service, int new_fd);

/**
 * Handles the death of the services. If the dead child was an UDP service,
 * unblocks it's socket descriptor.
 */
void signal_handler(int sig);
