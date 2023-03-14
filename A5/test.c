// #include <stdio.h>
// #include <pthread.h>
// #include <unistd.h>

// void *thread1(void *arg) {
//     while (1) {
//         printf("Thread 1 is running\n");
//         sleep(2);
//     }
//     return NULL;
// }

// void *thread2(void *arg) {
//     while (1) {
//         printf("Thread 2 is running\n");
//         sleep(2);
//     }
//     return NULL;
// }

// int main() {
//     pthread_t t1, t2;

//     // Create the first thread and detach it
//     if(pthread_create(&t1, NULL, thread1, NULL)) {
//         fprintf(stderr, "Error creating thread 1\n");
//         return 1;
//     }
//     pthread_detach(t1);

//     // Create the second thread and detach it
//     if(pthread_create(&t2, NULL, thread2, NULL)) {
//         fprintf(stderr, "Error creating thread 2\n");
//         return 1;
//     }
//     pthread_detach(t2);

//     // Main program continues without waiting for the threads to finish
//     printf("Threads are running in the background...\n");
//     while(1) {}  // Infinite loop to keep the program running

//     return 0;
// }


#include <stdio.h>
#include <pthread.h>

// Declare a global mutex variable
pthread_mutex_t mutex;

// Shared variable to be protected by the mutex
int shared_var = 0;

void *thread1(void *arg) {
    // Acquire the mutex lock
    pthread_mutex_lock(&mutex);

    // Modify the shared variable
    shared_var++;
    printf("Thread 1: shared_var = %d\n", shared_var);

    // Release the mutex lock
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *thread2(void *arg) {
    // Acquire the mutex lock
    pthread_mutex_lock(&mutex);

    // Modify the shared variable
    shared_var++;
    printf("Thread 2: shared_var = %d\n", shared_var);

    // Release the mutex lock
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    pthread_t t1, t2;

    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // Create the first thread
    if(pthread_create(&t1, NULL, thread1, NULL)) {
        fprintf(stderr, "Error creating thread 1\n");
        return 1;
    }

    // Create the second thread
    if(pthread_create(&t2, NULL, thread2, NULL)) {
        fprintf(stderr, "Error creating thread 2\n");
        return 1;
    }

    // Wait for both threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    printf("Both threads have finished\n");

    return 0;
}
