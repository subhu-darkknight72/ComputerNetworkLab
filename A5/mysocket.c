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

#define PORT 20000
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

void *recv_thread(void *arg)
{
    while (1)
    {
        // printf("recv_thread\n");

        char buf[MAXLEN];

        memset(buf, 0, MAXLEN);
        recv(*(int *)arg, buf, MAXLEN, 0);
        // printf("RECV_THREAD: %s\n", buf);

        // mssg_table_write(rmt, buf);
        int pos = rmt->write_ptr;
        rmt->table[pos] = (char *)calloc(MAXLEN, sizeof(char));
        strcpy(rmt->table[pos], buf);

        rmt->write_ptr = (pos + 1) % BUF_SIZE;
        rmt->size = rmt->size + 1;

        break;
    } 
    return NULL;
}
void *send_thread(void *arg)
{
    while (1)
    {
        // printf("send_thread\n");
        char buf[BUF_SIZE];
        char mssg[MAXLEN];

        // read from smt
        // if(smt->size == 0)  continue;

        // mssg_table_read(smt, buf);
        int pos = smt->read_ptr;
        strcpy(buf, smt->table[pos]);
        // printf("READ smt->table[%d] = %s\n", pos, buf);
        smt->read_ptr = (pos + 1) % BUF_SIZE;
        smt->size = smt->size - 1;
        // printf("buf: %s\n", buf);
        // printf("SEND_Thread read_ptr=%d, size=%d \n", smt->read_ptr, smt->size);

        int n = send(*(int *)arg, buf, strlen(buf)+1, 0);
        if(n<0)
            perror("send");

        printf("send %d bytes\n", n);
        break;
    }
    return NULL;
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

    printf("my_socket: sockfd=%d\n",sockfd);
    return sockfd;
}

int my_send(int sockfd_id, const void *buf, size_t len, int flags)
{
    // printf("my_send\n");
    // mssg_table_write(smt, (char *)buf);
    int pos = smt->write_ptr;
    smt->table[pos] = (char *)calloc(MAXLEN, sizeof(char));
    strcpy(smt->table[pos], buf);
    // printf("WRTIE smt->table[%d] = %s\n", pos, smt->table[pos]);

    smt->write_ptr = (pos + 1) % BUF_SIZE;
    smt->size = smt->size + 1;
    // printf("MY_SEND: write=%d, size=%d \n", smt->write_ptr, smt->size);

    int *arg = (int *)malloc(sizeof(int));
    *arg = sockfd_id;
    send_thread(arg);
    
    // printf("my_send end\n");
    return len;
}

ssize_t my_recv(int sockfd_id, void *buf, size_t len, int flags)
{
    // printf("my_recv\n");

    int *arg = (int *)malloc(sizeof(int));
    *arg = sockfd_id;
    recv_thread(arg);
    // mssg_table_read(rmt, (char *)buf);
    int pos = rmt->read_ptr;
    strcpy(buf, rmt->table[pos]);
    // printf("READ rmt->table[%d] = %s\n", pos, rmt->table[pos]);

    rmt->read_ptr = (pos + 1) % BUF_SIZE;
    rmt->size = rmt->size - 1;
    return len;
}

int my_close(int sockfd)
{
    // destroy the mssg_table
    // free(smt);
    // free(rmt);

    return close(sockfd);
}

int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return bind(sockfd, addr, addrlen);
}

int my_listen(int sockfd, int backlog)
{
    return listen(sockfd, backlog);
}

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    return accept(sockfd, addr, addrlen);
}

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return connect(sockfd, addr, addrlen);
}
