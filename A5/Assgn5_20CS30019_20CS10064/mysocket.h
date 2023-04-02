#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCK_MyTCP 1964

int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_listen(int sockfd, int backlog);
int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_send(int sockfd, const void *buf, size_t len, int flags);
int my_recv(int sockfd, void *buf, size_t len, int flags);
int my_close(int sockfd);

//  to send and recieve messages from the socket in packets of size BUF_SIZE
void sendStr(char *str, int socket_id);
void receiveStr(char *str, int socket_id);

// my_socket – This function opens a standard TCP socket with the socket call. It also creates two threads R and S (to be described later), allocates and initializes space for two tables Send_Message and Received_Message (to be described later), and any additional space that may be needed. The parameters to these are the same as the normal socket( ) call, except that it will take only SOCK_MyTCP as the socket type.
// my_bind – binds the socket with some address-port using the bind call.
// my_listen – makes a listen call.
// my_accept – accepts a connection on the MyTCP socket by making the accept call on the TCP socket (only on server side)
// my_connect – opens a connection through the MyTCP socket by making the connect call on the TCP socket
// my_send – sends a message (see description later). One message is defined as what is sent in one my_send call.
// my_recv – receives a message. This is a blocking call and will return only if a message, sent using the my_send call, is received. If the buffer length specified is less than the message length, the extra bytes in the message are discarded (similar to that in UDP).
// my_close – closes the socket and cleans up everything. If there are any messages to be sent/received, they are discarded.


