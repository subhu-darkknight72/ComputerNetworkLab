#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// include header for thread
#include <pthread.h>

// include header for mutex
#include <pthread.h>

// include header for semaphore
#include <semaphore.h>

// include header for errno
#include <errno.h>

#define PORT 8080
#define MAXLEN 1024

const int BUF_SIZE = 1024;
// vector<int> mssg_table;

typedef struct mssg_table
{
    int write_ptr;
    int read_ptr;
    int size;
    char **table;
} mssg_table;

int sockfd;
pthread_t R, S;
pthread_t p[2];
mssg_table *smt;
mssg_table *rmt;
pthread_mutex_t recvLock, sendLock, msgSendLock;
pthread_cond_t cond1, cond2;

mssg_table *mssg_table_init(int size)
{
    mssg_table *mt = (mssg_table *)calloc(1, sizeof(mssg_table));
    mt->write_ptr = 0;
    mt->read_ptr = 0;
    mt->size = 0;
    mt->table = (char **)calloc(BUF_SIZE, sizeof(char *));
    return mt;
}

void mssg_table_read(mssg_table *mt, char *mssg)
{
    printf("in mssg_table_read\n");
    int pos = mt->read_ptr;
    // printf("pos = %d\n",pos);
    strcpy(mssg, mt->table[pos]);
    printf("READ mt->table[%d] = %s\n", pos, mt->table[pos]);

    mt->read_ptr = (pos + 1) % BUF_SIZE;
    mt->size = mt->size - 1;
}

void mssg_table_write(mssg_table *mt, char *mssg)
{
    printf("in mssg_table_write\n");
    int pos = mt->write_ptr;

    mt->table[pos] = (char *)calloc(MAXLEN, sizeof(char));
    strcpy(mt->table[pos], mssg);

    printf("WRTIE smt->table[%d] = %s\n", pos, mt->table[pos]);

    mt->write_ptr = (pos + 1) % BUF_SIZE;
    mt->size = mt->size + 1;
}

void *recv_thread(void *arg)
{
    while (1)
    {
        sleep(1);
        if(rmt->size == BUF_SIZE)  continue;

        char buf[MAXLEN];

        pthread_mutex_lock(&recvLock);
        // read message from server
        recv(sockfd, buf, MAXLEN, 0);
        printf("buf = %s\n", buf);

        // mssg_table_write(rmt, buf);
        int pos = rmt->write_ptr;
        rmt->table[pos] = (char *)calloc(MAXLEN, sizeof(char));
        strcpy(rmt->table[pos], buf);

        rmt->write_ptr = (pos + 1) % BUF_SIZE;
        rmt->size = rmt->size + 1;

        pthread_mutex_unlock(&recvLock);
    }
}
void *send_thread(void *arg)
{
    while (1)
    {
        sleep(1);

        char buf[BUF_SIZE];
        char mssg[MAXLEN];

        // read from smt
        if(smt->size == 0)  continue;

        pthread_mutex_lock(&sendLock);
        // mssg_table_read(smt, buf);
        int pos = smt->read_ptr;
        printf("READ mt->table[%d] = %s\n", pos, smt->table[pos]);
        smt->read_ptr = (pos + 1) % BUF_SIZE;
        smt->size = smt->size - 1;

        send(sockfd, buf, BUF_SIZE, 0);
        pthread_mutex_unlock(&sendLock);

    }
}

int my_socket(int domain, int type, int protocol)
{
    if ((sockfd = socket(domain, type, protocol)) < 0)
    {
        perror("Cannot create socket\n");
        exit(0);
    }
    smt = mssg_table_init(BUF_SIZE);
    rmt = mssg_table_init(BUF_SIZE);

    // pthread_create(&R,0,recv_thread,0);
    // pthread_create(&S,0,send_thread,0);
    for (int i = 0; i < 2; i++)
    {
        if (i == 0)
        {
            pthread_create(&p[i], 0, recv_thread, 0);
            pthread_detach(p[i]);
        }
        else
        {
            pthread_create(&p[i], 0, send_thread, 0);
            pthread_detach(p[i]);
        }
    }
    // for(int i=0;i<2;i++){
    //     pthread_join(p[i],0);
    // }
    // pthread_join(R,0);
    // pthread_join(S,0);
    return sockfd;
}

int my_send(int sockfd_id, const void *buf, size_t len, int flags)
{
    // search for socket_desc
    if (sockfd_id != sockfd)
    {
        printf("Socket not found\n");
        return -1;
    }

    while (smt->size == BUF_SIZE)
    {
        sleep(1);
    }

    pthread_mutex_lock(&sendLock);

    // mssg_table_write(smt, (char *)buf);
    int pos = smt->write_ptr;
    smt->table[pos] = (char *)calloc(MAXLEN, sizeof(char));
    strcpy(smt->table[pos], buf);
    printf("WRTIE smt->table[%d] = %s\n", pos, smt->table[pos]);

    smt->write_ptr = (pos + 1) % BUF_SIZE;
    smt->size = smt->size + 1;

    pthread_mutex_unlock(&sendLock);

    return len;
}

ssize_t my_recv(int sockfd_id, void *buf, size_t len, int flags)
{
    // search for socket_desc
    if (sockfd_id != sockfd)
    {
        printf("Socket not found\n");
        return -1;
    }

    // read from mssg_table
    while (rmt->size == 0)
        sleep(1);

    pthread_mutex_lock(&recvLock);
    // mssg_table_read(rmt, (char *)buf);
    int pos = rmt->read_ptr;
    strcpy(buf, rmt->table[pos]);
    printf("READ rmt->table[%d] = %s\n", pos, rmt->table[pos]);

    rmt->read_ptr = (pos + 1) % BUF_SIZE;
    rmt->size = rmt->size - 1;
    pthread_mutex_unlock(&recvLock);

    return len;
}

int my_close(int sockfd)
{
    // destroy the threads
    pthread_exit(&R);
    pthread_exit(&S);

    // destroy the mssg_table
    free(smt);
    free(rmt);

    return close(sockfd);
}

int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return bind(sockfd, addr, addrlen);
    // int ret = bind(sockfd, addr, addrlen);
    // if (ret < 0) {
    //     perror("Unable to bind local address\n");
    //     exit(0);
    // }
    // return ret;
}

int my_listen(int sockfd, int backlog)
{
    return listen(sockfd, backlog);
    // int ret = listen(sockfd, backlog);
    // if (ret < 0) {
    //     perror("Unable to listen\n");
    //     exit(0);
    // }
    // return ret;
}

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    return accept(sockfd, addr, addrlen);

    // int newsockfd;
    // newsockfd = accept(sockfd, addr, addrlen);
    // return newsockfd;
}

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
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
