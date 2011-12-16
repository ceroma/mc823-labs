#include "sv_utils.h"

#define MAX_NAME_SIZE 20
#define MAX_PATH_SIZE 100
#define MAX_LINE_SIZE 200

#define SPACE_DELIMS " \t\n"

#define MYINETD_CONF_PATH "/home/kerhott/lab5/myinetd.conf"

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
    char **args;
} service_t;

/**
 * List of services.
 */
typedef struct {
    int N;
    service_t * service;
} services_t;

/**
 * Node of execution list.
 */
struct list_node {
    int id;
    int pid;
    int service;
    struct list_node * next;
};

/**
 * List of executed services.
 */
typedef struct {
    int N;
    struct list_node * head;
} list_t;

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
 * Logs the service and ID that will start running.
 */
void log_service_start(char *service_name);

/**
 * Logs the service and ID that just stopped.
 */
void log_service_stop(char *service_name, int id);

/**
 * Associates a recently executed service with an internal ID and adds it to
 * the execution list.
 */
void list_add(int pid, int service);

/**
 * Executes a given service.
 */
void execute_service(service_t service, int new_fd);

/**
 * Handles the death of the services. If the dead child was an UDP service,
 * unblocks it's socket descriptor.
 */
void signal_handler(int sig);
