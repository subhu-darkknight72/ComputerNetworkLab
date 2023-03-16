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
// include header for errno
#include <errno.h>

#define PORT 8080
#define SOCK_MyTCP 1964

const int MAXLEN = 10240;
const int BUF_SIZE = 256;

typedef struct mssg_table
{
    int write_ptr;
    int read_ptr;
    int size;
    char **table;
    int *lens;
} mssg_table;

int sockfd;
pthread_t R, S;
mssg_table *smt;
mssg_table *rmt;

pthread_mutex_t recvLock, sendLock;
pthread_cond_t cond_full_smt, cond_empty_smt;
pthread_cond_t cond_full_rmt, cond_empty_rmt;

mssg_table *mssg_table_init(int size)
{
    mssg_table *mt = (mssg_table *)calloc(1, sizeof(mssg_table));
    mt->write_ptr = 0;
    mt->read_ptr = 0;
    mt->size = 0;
    mt->table = (char **)calloc(BUF_SIZE, sizeof(char *));
    mt->lens = (int *)calloc(BUF_SIZE, sizeof(int));
    return mt;
}

void *recv_thread(void *arg)
{
    while (1)
    {
        sleep(5);
        pthread_mutex_lock(&recvLock);
        while (rmt->size == BUF_SIZE || sockfd == -1)
            pthread_cond_wait(&cond_full_rmt, &recvLock);

        int len = 0, temp = 0;
        do{
            temp = recv(sockfd, &len, sizeof(len), MSG_PEEK);
        }while(temp < sizeof(len));
        recv(sockfd, &len, sizeof(len), 0);
        // printf("len=%d\n", len);

        char mssg[MAXLEN], buf[BUF_SIZE];
        memset(mssg, 0, MAXLEN);
        memset(buf, 0, BUF_SIZE);

        int total_recv=0, n;
        while(total_recv<len){
            n = recv(sockfd, mssg+total_recv, len-total_recv, 0);
            if(n<0){
                perror("recv");
                pthread_exit(NULL);
            }
            if(n==0){
                printf("server closed\n");
                pthread_exit(NULL);
            }
            total_recv += n;
        }
        
        // mssg_table_write(rmt, buf);
        int pos = rmt->write_ptr;
        rmt->table[pos] = (char *)calloc(MAXLEN, sizeof(char));
        strcpy(rmt->table[pos], mssg);
        rmt->lens[pos] = len;

        rmt->write_ptr = (pos + 1) % BUF_SIZE;
        rmt->size = rmt->size + 1;

        pthread_mutex_unlock(&recvLock);
        pthread_cond_signal(&cond_empty_rmt);
    }
    return 0;
}
void *send_thread(void *arg)
{
    while (1)
    {
        sleep(5);
        pthread_mutex_lock(&sendLock);
        while (smt->size == 0 || sockfd == -1)
            pthread_cond_wait(&cond_empty_smt, &sendLock);

        char buf[BUF_SIZE], mssg[MAXLEN];
        memset(buf, '\0', BUF_SIZE);
        memset(mssg, '\0', MAXLEN);

        // mssg_table_read(smt, buf);
        int pos = smt->read_ptr;
        strcpy(buf, smt->table[pos]);
        smt->read_ptr = (pos + 1) % BUF_SIZE;
        smt->size = smt->size - 1;
        int len = smt->lens[pos];

        int total_sent, n;
        send(sockfd, &len, sizeof(len), 0);
        // perror("send");

        total_sent=0;
        while(total_sent<len){
            // printf("sockfd=%d buf=%s, len=%d\n", sockfd, buf, len);
            n = send(sockfd, buf+total_sent, len-total_sent, 0);
            // printf("send %d bytes\n", n);
            if(n<0){
                perror("send");
                // pthread_exit(NULL);
                exit(1);
            }
            if(n==0){
                printf("server closed\n");
                // pthread_exit(NULL);
                exit(1);
            }
            total_sent += n;
        }

        pthread_mutex_unlock(&sendLock);
        pthread_cond_signal(&cond_full_smt);
    }
    return 0;
}

int my_socket(int domain, int type, int protocol)
{
    if(type != SOCK_MyTCP){
        printf("Not mytcp socket\n");
        exit(0);
    }

    if ((sockfd = socket(domain, SOCK_STREAM, protocol)) < 0)
    {
        perror("Cannot create socket\n");
        exit(0);
    }

    smt = mssg_table_init(BUF_SIZE);
    rmt = mssg_table_init(BUF_SIZE);

    recvLock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    sendLock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    cond_full_smt = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    cond_empty_smt = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    cond_full_rmt = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    cond_empty_rmt = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    // create threads for send and recv and join them
    pthread_create(&R, NULL, recv_thread, NULL);
    pthread_create(&S, NULL, send_thread, NULL);

    printf("my_socket: sockfd=%d\n", sockfd);
    return sockfd;
}

int my_send(int sockfd_id, const void *buf, size_t len, int flags)
{
    pthread_mutex_lock(&sendLock);
    while (smt->size == BUF_SIZE)
        pthread_cond_wait(&cond_full_smt, &sendLock);
    

    // mssg_table_write(smt, (char *)buf);
    int pos = smt->write_ptr;
    smt->table[pos] = (char *)calloc(MAXLEN, sizeof(char));
    strcpy(smt->table[pos], buf);
    smt->lens[pos] = len;

    smt->write_ptr = (pos + 1) % BUF_SIZE;
    smt->size = smt->size + 1;

    sockfd = sockfd_id;
    
    pthread_mutex_unlock(&sendLock);
    pthread_cond_signal(&cond_empty_smt);

    return len;
}

ssize_t my_recv(int sockfd_id, void *buf_in, size_t len_in, int flags)
{
    char *buf = (char *)buf_in;
    pthread_mutex_lock(&recvLock);
    while (rmt->size == 0)
        pthread_cond_wait(&cond_empty_rmt, &recvLock);

    // mssg_table_read(rmt, (char *)buf);
    sockfd = sockfd_id;
    int pos = rmt->read_ptr;
    strcpy(buf, rmt->table[pos]);
    int len = rmt->lens[pos];

    rmt->read_ptr = (pos + 1) % BUF_SIZE;
    rmt->size = rmt->size - 1;

    pthread_mutex_unlock(&recvLock);
    pthread_cond_signal(&cond_full_rmt);

    return len;
}

int my_close(int sockfd)
{
    sleep(15);
    // destroy the threads
    pthread_cancel(R);
    pthread_cancel(S);

    pthread_join(R, NULL);
    pthread_join(S, NULL);

    // destroy the mssg_table
    free(smt);
    free(rmt);

    return close(sockfd);
}

int my_bind(int sockfd_in, const struct sockaddr *addr, socklen_t addrlen)
{
    return bind(sockfd_in, addr, addrlen);
}

int my_listen(int sockfd_in, int backlog)
{
    return listen(sockfd_in, backlog);
}

int my_accept(int sockfd_in, struct sockaddr *addr, socklen_t *addrlen)
{
    sockfd = accept(sockfd, addr, addrlen);
    pthread_cond_signal(&cond_full_rmt);
    pthread_cond_signal(&cond_empty_smt);
    return sockfd;
}

int my_connect(int sockfdin, const struct sockaddr *addr, socklen_t addrlen)
{
    sockfd = sockfdin;
    return connect(sockfdin, addr, addrlen);
}

void sendStr(char *str, int socket_id)
{
    int pos, i, len = strlen(str);
    char buf[BUF_SIZE];

    for (pos = 0; pos < len; pos += BUF_SIZE)
    {
        for (i = 0; i < BUF_SIZE; i++)
            buf[i] = ((pos + i) < len) ? str[pos + i] : '\0';

        if (send(socket_id, buf, BUF_SIZE, 0) < 0)
        {
            perror("error in transmission.\n");
            exit(-1);
        }
    }
}

void receiveStr(char *str, int socket_id)
{
    int flag = 0, i, pos = 0;
    char buf[BUF_SIZE];
    while (flag == 0)
    {
        // recv(socket_id, buf, BUF_SIZE, 0)
        if (recv(socket_id, buf, BUF_SIZE, 0) < 0)
        {
            perror("error in transmission.\n");
            exit(-1);
        }

        for (i = 0; i < BUF_SIZE && flag == 0; i++)
            if (buf[i] == '\0')
                flag = 1;

        for (i = 0; i < BUF_SIZE; i++, pos++)
            str[pos] = buf[i];
    }
}