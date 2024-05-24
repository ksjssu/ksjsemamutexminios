#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/resource.h>
#include "semaphore.h"
#include "local_mutex.h"

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 3
#define NUM_ITEMS 10

typedef struct {
    int buffer[BUFFER_SIZE];
    int count;
    int in;
    int out;
    semaphore_t empty_sem;
    semaphore_t full_sem;
    local_mutex_t mutex;
} bounded_buffer_t;

bounded_buffer_t waterfall_buffer;

int context_switches_waterfall = 0;
double total_waiting_time_producers_waterfall = 0.0;
double total_waiting_time_consumers_waterfall = 0.0;
double total_response_time_producers_waterfall = 0.0;
double total_response_time_consumers_waterfall = 0.0;
struct timeval start_time_waterfall, end_time_waterfall;
struct rusage usage_waterfall;

void buffer_init_waterfall(bounded_buffer_t *b) {
    b->count = 0;
    b->in = 0;
    b->out = 0;
    semaphore_init(&b->empty_sem, BUFFER_SIZE);
    semaphore_init(&b->full_sem, 0);
    local_mutex_init(&b->mutex);
}

void buffer_destroy_waterfall(bounded_buffer_t *b) {
    semaphore_destroy(&b->empty_sem);
    semaphore_destroy(&b->full_sem);
    local_mutex_destroy(&b->mutex);
}

void buffer_insert_waterfall(bounded_buffer_t *b, int item, double *wait_time) {
    struct timeval wait_start, wait_end;
    gettimeofday(&wait_start, NULL);

    semaphore_wait(&b->empty_sem);

    gettimeofday(&wait_end, NULL);
    *wait_time += (wait_end.tv_sec - wait_start.tv_sec) + (wait_end.tv_usec - wait_start.tv_usec) / 1.0e6;

    local_mutex_lock(&b->mutex);
    b->buffer[b->in] = item;
    b->in = (b->in + 1) % BUFFER_SIZE;
    b->count++;
    printf("Produced: %d\n", item);
    local_mutex_unlock(&b->mutex);

    semaphore_signal(&b->full_sem);
    context_switches_waterfall++;
}

int buffer_remove_waterfall(bounded_buffer_t *b, double *wait_time) {
    struct timeval wait_start, wait_end;
    int item;

    gettimeofday(&wait_start, NULL);

    semaphore_wait(&b->full_sem);

    gettimeofday(&wait_end, NULL);
    *wait_time += (wait_end.tv_sec - wait_start.tv_sec) + (wait_end.tv_usec - wait_start.tv_usec) / 1.0e6;

    local_mutex_lock(&b->mutex);
    item = b->buffer[b->out];
    b->out = (b->out + 1) % BUFFER_SIZE;
    b->count--;
    printf("Consumed: %d\n", item);
    local_mutex_unlock(&b->mutex);

    semaphore_signal(&b->empty_sem);
    context_switches_waterfall++;

    return item;
}

void* producer_waterfall(void* param) {
    int i;
    double wait_time = 0.0;
    struct timeval response_start, response_end;

    for (i = 0; i < NUM_ITEMS; i++) {
        gettimeofday(&response_start, NULL);
        buffer_insert_waterfall(&waterfall_buffer, i, &wait_time);
        gettimeofday(&response_end, NULL);
        total_response_time_producers_waterfall += (response_end.tv_sec - response_start.tv_sec) + 
                                         (response_end.tv_usec - response_start.tv_usec) / 1.0e6;
    }

    total_waiting_time_producers_waterfall += wait_time;
    return NULL;
}

void* consumer_waterfall(void* param) {
    int i, item;
    double wait_time = 0.0;
    struct timeval response_start, response_end;

    for (i = 0; i < NUM_ITEMS; i++) {
        gettimeofday(&response_start, NULL);
        item = buffer_remove_waterfall(&waterfall_buffer, &wait_time);
        gettimeofday(&response_end, NULL);
        total_response_time_consumers_waterfall += (response_end.tv_sec - response_start.tv_sec) + 
                                         (response_end.tv_usec - response_start.tv_usec) / 1.0e6;
    }

    total_waiting_time_consumers_waterfall += wait_time;
    return NULL;
}

void semamutexproduce() {
    int i;
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    double total_time_waterfall, avg_wait_time_producers_waterfall, avg_wait_time_consumers_waterfall;
    double avg_response_time_producers_waterfall, avg_response_time_consumers_waterfall;

    buffer_init_waterfall(&waterfall_buffer);
    gettimeofday(&start_time_waterfall, NULL);

    for (i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer_waterfall, NULL);
    }
    for (i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer_waterfall, NULL);
    }

    for (i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    gettimeofday(&end_time_waterfall, NULL);
    buffer_destroy_waterfall(&waterfall_buffer);

    total_time_waterfall = (end_time_waterfall.tv_sec - start_time_waterfall.tv_sec) + (end_time_waterfall.tv_usec - start_time_waterfall.tv_usec) / 1.0e6;
    avg_wait_time_producers_waterfall = total_waiting_time_producers_waterfall / (NUM_PRODUCERS * NUM_ITEMS);
    avg_wait_time_consumers_waterfall = total_waiting_time_consumers_waterfall / (NUM_CONSUMERS * NUM_ITEMS);
    avg_response_time_producers_waterfall = total_response_time_producers_waterfall / (NUM_PRODUCERS * NUM_ITEMS);
    avg_response_time_consumers_waterfall = total_response_time_consumers_waterfall / (NUM_CONSUMERS * NUM_ITEMS);

    getrusage(RUSAGE_SELF, &usage_waterfall);

    printf("Total Time: %.6f seconds\n", total_time_waterfall);
    printf("Context Switches: %d\n", context_switches_waterfall);
    printf("Average Waiting Time (Producers): %.6f seconds\n", avg_wait_time_producers_waterfall);
    printf("Average Waiting Time (Consumers): %.6f seconds\n", avg_wait_time_consumers_waterfall);
    printf("Average Response Time (Producers): %.6f seconds\n", avg_response_time_producers_waterfall);
    printf("Average Response Time (Consumers): %.6f seconds\n", avg_response_time_consumers_waterfall);
    printf("Maximum Resident Set Size: %ld KB\n", usage_waterfall.ru_maxrss);
    printf("Producer Waiting Time: %.6f seconds\n", total_waiting_time_producers_waterfall);
    printf("Consumer Waiting Time: %.6f seconds\n", total_waiting_time_consumers_waterfall);

    
}
