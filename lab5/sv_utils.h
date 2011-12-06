/**
 * Transport protocol enum: TCP or UDP.
 */
typedef enum {TCP, UDP} socktype_t;

/**
 * Returns a TCP socket listening to connections from the given port or an UDP
 * datagram.
 */
int tcpudp_socket(int port, socktype_t type);

/**
 * Returns an UDP datagram.
 */
int udp_socket(int port);

/**
 * Returns a TCP socket listening to connections from the given port.
 */
int tcp_socket(int port);

/**
 * Waits for a new connection request and logs the connector's address. Returns
 * a file descriptor referring to the accepted socket, if TCP.
 */
int tcpudp_accept(int sockfd, socktype_t type);

/**
 * Waits for a new connection request and returns a file descriptor referring
 * to the accepted socket.
 */
int tcp_accept(int sockfd);

/**
 * Waits for an UDP datagram and logs the connector's address.
 */
int udp_accept(int sockfd);
