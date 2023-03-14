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

int user_cnt = 0;
pthread_cond_t cond;
pthread_mutex_t mutex;

void *myThreadFun(void *vargp)
{
    pthread_mutex_lock(&mutex);
    while (user_cnt < 1000)
    {
        // sleep(1);
        user_cnt += rand() % 100;
        printf("%d : ", user_cnt);
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_exit(0);
    return vargp;
}

int main()
{
    pthread_t thread_id;
    printf("Before Thread\n");

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_mutex_init(&mutex, 0);

    pthread_create(&thread_id, &attr, myThreadFun, 0);
    pthread_join(thread_id, 0);
    // int ret_val;
    // pthread_join(thread_id, &ret_val);
    // printf("Thread returned %d\n", ret_val);

    printf("After Thread\n");
    pthread_mutex_lock(&mutex);
    while (user_cnt < 1000)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    printf("%d\n", user_cnt);
    pthread_mutex_unlock(&mutex);

    exit(0);
}