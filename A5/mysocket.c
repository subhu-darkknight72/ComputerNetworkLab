#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

const int BUF_SIZE = 1024;
// vector<int> mssg_table;

typedef struct mssg_table{
    int write_ptr;
    int read_ptr;
    int size;
    char **table;
} mssg_table;

mssg_table* mssg_table_init(int size){
    mssg_table *smt = (mssg_table *)calloc(1,sizeof(mssg_table));
    smt->write_ptr = 0;
    smt->read_ptr = 0;
    smt->size = 0;
    smt->table = (char **)calloc(BUF_SIZE, sizeof(char *));
    return smt;
}

void mssg_table_read(mssg_table *smt, char *mssg){
    while(smt->size == 0);

    int pos = smt->read_ptr;
    strcpy(smt->table[pos],mssg);
    printf("READ smt->table[%d] = %s",pos,smt->table[pos]);

    smt->read_ptr = (pos+1) % BUF_SIZE;
    smt->size = smt->size - 1;
}

void mssg_table_write(mssg_table *smt, char *mssg){
    while(smt->size == BUF_SIZE);

    int pos = smt->write_ptr;
    strcpy(mssg,smt->table[pos]);
    printf("WRTIE smt->table[%d] = %s",pos, smt->table[pos]);
    
    smt->write_ptr = (pos+1) % BUF_SIZE;
    smt->size = smt->size + 1;
}

typedef struct socket_desc{
    int sockfd;
    int domain;
    int type;
    int protocol;
    struct sockaddr_in *addr;
    socklen_t addrlen;
    int backlog;
    int flags;
    int ret;
    int n;
    char *buf;
    size_t len;

    mssg_table *smt;
} socket_desc;

int my_socket(int domain, int type, int protocol){
    int sockfd;
    if ((sockfd = socket(domain, type, protocol)) < 0) {
        perror("Cannot create socket\n");
        exit(0);
    }
    return sockfd;
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

ssize_t my_recv(int sockfd, void *buf, size_t len, int flags){
    ssize_t n =  recvfrom(sockfd, buf, len, flags, NULL, NULL);
    return n;
}

int my_close(int sockfd){
    return close(sockfd);
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




// void sendStr(char* str, int socket_id){
//     int pos, i, len=strlen(str);
//     char buf[BUF_SIZE];

//     for(pos=0; pos<len; pos+=BUF_SIZE){
//         for(i=0; i<BUF_SIZE; i++)
//             buf[i] = ((pos+i)<len) ? str[pos+i]: '\0';

// 		// send(socket_id, buf, BUF_SIZE, 0);
//         if(send(socket_id, buf, BUF_SIZE, 0) < 0){
//             perror("error in transmission.\n");
//             exit(-1);
//         }
//     }
// }

// void receiveStr(char *str, int socket_id){
//     int flag=0, i, pos=0;
//     char buf[BUF_SIZE];
//     while(flag==0){
//         // recv(socket_id, buf, BUF_SIZE, 0)
// 		if( recv(socket_id, buf, BUF_SIZE, 0) < 0 ){
//             perror("error in transmission.\n");
//             exit(-1);
//         }
// 		// printf("$%s$\n",buf);

//         for(i=0; i<BUF_SIZE && flag==0; i++)
//             if(buf[i]=='\0') 
// 				flag=1;

//         for(i=0; i<BUF_SIZE; i++, pos++)	
// 			str[pos] = buf[i];
//     }
// }


