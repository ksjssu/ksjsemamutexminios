#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "semaphore.h"


#define BUFFER_SIZE 5
#define NUM_ITEMS 10

static int buffer[BUFFER_SIZE];
static int in = 0;
static int out = 0;
static semaphore_t empty, full, mutex;

static void *producer(void *arg) {
    int item;
    struct timeval start, end;
    long long wait_time = 0;
    struct rusage usage;

    for (int i = 0; i < NUM_ITEMS; ++i) {
        item = rand() % 100;
        gettimeofday(&start, NULL);

        semaphore_wait(&empty);
        semaphore_wait(&mutex);

        buffer[in] = item;
        printf("Producer produced item %d at index %d\n", item, in);
        in = (in + 1) % BUFFER_SIZE;

        semaphore_signal(&mutex);
        semaphore_signal(&full);

        gettimeofday(&end, NULL);
        wait_time += (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
    }

    printf("Producer total wait time: %lld microseconds\n", wait_time);

    // 리소스 사용량 측정
    getrusage(RUSAGE_SELF, &usage);
    printf("Producer maximum resident set size: %ld kilobytes\n", usage.ru_maxrss);
    printf("Producer context switches: %ld\n", usage.ru_nvcsw + usage.ru_nivcsw);

    pthread_exit(NULL);
}

static void *consumer(void *arg) {
    int item;
    struct timeval start, end;
    long long wait_time = 0;
    struct rusage usage;

    for (int i = 0; i < NUM_ITEMS; ++i) {
        gettimeofday(&start, NULL);

        semaphore_wait(&full);
        semaphore_wait(&mutex);

        item = buffer[out];
        printf("Consumer consumed item %d from index %d\n", item, out);
        out = (out + 1) % BUFFER_SIZE;

        semaphore_signal(&mutex);
        semaphore_signal(&empty);

        gettimeofday(&end, NULL);
        wait_time += (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
    }

    printf("Consumer total wait time: %lld microseconds\n", wait_time);

    // 리소스 사용량 측정
    getrusage(RUSAGE_SELF, &usage);
    printf("Consumer maximum resident set size: %ld kilobytes\n", usage.ru_maxrss);
    printf("Consumer context switches: %ld\n", usage.ru_nvcsw + usage.ru_nivcsw);

    pthread_exit(NULL);
}

void produce() {
    pthread_t producer_thread, consumer_thread;
    struct rusage usage;

    // 세마포어 초기화
    semaphore_init(&empty, BUFFER_SIZE);
    semaphore_init(&full, 0);
    semaphore_init(&mutex, 1);

    // 스레드 생성
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // 스레드 종료 대기
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // 메인 스레드의 리소스 사용량 측정
    getrusage(RUSAGE_SELF, &usage);
    printf("Main thread maximum resident set size: %ld kilobytes\n", usage.ru_maxrss);
    printf("Main thread context switches: %ld\n", usage.ru_nvcsw + usage.ru_nivcsw);

    // 세마포어 제거
    semaphore_destroy(&empty);
    semaphore_destroy(&full);
    semaphore_destroy(&mutex);

   
}

