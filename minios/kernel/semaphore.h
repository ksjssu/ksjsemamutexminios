#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <pthread.h>

typedef struct {
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} semaphore_t;

void semaphore_init(semaphore_t *sem, int value);
void semaphore_wait(semaphore_t *sem);
void semaphore_signal(semaphore_t *sem);
void semaphore_destroy(semaphore_t *sem);

#endif // SEMAPHORE_H

