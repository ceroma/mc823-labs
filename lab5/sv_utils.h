/**
 * Returns a TCP socket listening to connections from the given port.
 */
int tcp_socket(int port);

/**
 * Waits for a new connection request and returns a file descriptor referring
 * to the accepted socket.
 */
int tcp_accept(int sockfd);
