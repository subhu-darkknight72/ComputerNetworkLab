#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

int my_socket(int domain, int type, int protocol){
    int sockfd;
    if ((sockfd = socket(domain, type, protocol)) < 0) {
        perror("Cannot create socket\n");
        exit(0);
    }
    return sockfd;
}

int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    return bind(sockfd, addr, addrlen);
    // int ret = bind(sockfd, addr, addrlen);
    // if (ret < 0) {
    //     perror("Unable to bind local address\n");
    //     exit(0);
    // }
    // return ret;
}

int my_listen(int sockfd, int backlog){
    return listen(sockfd, backlog);
    // int ret = listen(sockfd, backlog);
    // if (ret < 0) {
    //     perror("Unable to listen\n");
    //     exit(0);
    // }
    // return ret;
}

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    return accept(sockfd, addr, addrlen);

    // int newsockfd;
    // newsockfd = accept(sockfd, addr, addrlen);
    // return newsockfd;
}

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    return connect(sockfd, addr, addrlen);

    // int ret = connect(sockfd, addr, addrlen);
    // if (ret < 0) {
    //     perror("Unable to connect to remote host\n");
    //     exit(0);
    // }
    // return ret;
}

int my_send(int sockfd, const void *buf, size_t len, int flags){
    return send(sockfd, buf, len, flags);

    // int ret = send(sockfd, buf, len, flags);
    // if ( ret< 0) {
    //     perror("Unable to send data\n");
    //     exit(0);
    // }
    // return ret;
}

int my_recv(int sockfd, void *buf, size_t len, int flags){
    return recv(sockfd, buf, len, flags);

    // int ret = recv(sockfd, buf, len, flags);
    // if (ret < 0) {
    //     perror("Unable to receive data\n");
    //     exit(0);
    // }
    // return ret;
}

int my_close(int sockfd){
    return close(sockfd);
}

// Path: mysocket.h
// Compare this snippet from mysocket.c:
//
// #include "mysocket.h"
//
// int main(int argc, char *argv[]){
//     int sockfd;
//     sockfd = my_socket(AF_INET, SOCK_MyTCP, 0);
//     return 0;
// }
