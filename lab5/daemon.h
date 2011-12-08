#include <netinet/in.h>

/**
 * Daemonizes the server.
 */
void daemon_init();

/**
 * Logs message to a private file.
 */
void log_message(char *caller, char *message);

/**
 * Logs a new connection to the server.
 */
void log_conn(char *caller, struct sockaddr_in *in);

/**
 * Logs an execution error.
 */
void log_error(char *caller, char *fname);
