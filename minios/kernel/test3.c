/* 리눅스용
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

// test.c 헤더 파일 포함
#include "local_mutex.h"

#define BUFFER_SIZE 5
#define NUM_ITEMS 10

int buffer[BUFFER_SIZE];
int in = 0, out = 0;
local_mutex_t mutex, buffer_mutex;

void *producer(void *arg) {
    int item;
    struct timeval start, end;
    long long wait_time = 0;
    struct rusage usage;

    for (int i = 0; i < NUM_ITEMS; ++i) {
        item = rand() % 100;
        gettimeofday(&start, NULL);

        local_mutex_lock(&buffer_mutex);
        while ((in + 1) % BUFFER_SIZE == out) {
            local_mutex_unlock(&buffer_mutex);
            local_mutex_lock(&mutex);
            local_mutex_unlock(&mutex);
            local_mutex_lock(&buffer_mutex);
            gettimeofday(&end, NULL);
            wait_time += (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
            gettimeofday(&start, NULL);
        }

        buffer[in] = item;
        printf("Producer produced item %d at index %d\n", item, in);
        in = (in + 1) % BUFFER_SIZE;

        local_mutex_unlock(&buffer_mutex);
        local_mutex_lock(&mutex);
        local_mutex_unlock(&mutex);
    }

    printf("Producer total wait time: %lld microseconds\n", wait_time);

    // 리소스 사용량 측정
    getrusage(RUSAGE_THREAD, &usage);
    printf("Producer maximum resident set size: %ld kilobytes\n", usage.ru_maxrss);
    printf("Producer context switches: %ld\n", usage.ru_nvcsw + usage.ru_nivcsw);

    pthread_exit(NULL);
}

void *consumer(void *arg) {
    int item;
    struct timeval start, end;
    long long wait_time = 0;
    struct rusage usage;

    for (int i = 0; i < NUM_ITEMS; ++i) {
        gettimeofday(&start, NULL);
        local_mutex_lock(&buffer_mutex);
        while (in == out) {
            local_mutex_unlock(&buffer_mutex);
            local_mutex_lock(&mutex);
            local_mutex_unlock(&mutex);
            local_mutex_lock(&buffer_mutex);
            gettimeofday(&end, NULL);
            wait_time += (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
            gettimeofday(&start, NULL);
        }

        item = buffer[out];
        printf("Consumer consumed item %d from index %d\n", item, out);
        out = (out + 1) % BUFFER_SIZE;

        local_mutex_unlock(&buffer_mutex);
        local_mutex_lock(&mutex);
        local_mutex_unlock(&mutex);
    }

    printf("Consumer total wait time: %lld microseconds\n", wait_time);

    // 리소스 사용량 측정
    getrusage(RUSAGE_THREAD, &usage);
    printf("Consumer maximum resident set size: %ld kilobytes\n", usage.ru_maxrss);
    printf("Consumer context switches: %ld\n", usage.ru_nvcsw + usage.ru_nivcsw);

    pthread_exit(NULL);
}

int main() {
    pthread_t producer_thread, consumer_thread;
    struct rusage usage;

    // 뮤텍스 초기화
    local_mutex_init(&mutex);
    local_mutex_init(&buffer_mutex);

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

    // 뮤텍스 제거
    local_mutex_destroy(&mutex);
    local_mutex_destroy(&buffer_mutex);

    return 0;
} */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

// test.c 헤더 파일 포함
#include "local_mutex.h"

#define BUFFER_SIZE 5
#define NUM_ITEMS 10

int buffer[BUFFER_SIZE];
int in = 0, out = 0;
local_mutex_t mutex, buffer_mutex;

void *producer(void *arg) {
    int item;
    struct timeval start, end;
    long long wait_time = 0;
    struct rusage usage;

    for (int i = 0; i < NUM_ITEMS; ++i) {
        item = rand() % 100;
        gettimeofday(&start, NULL);
        local_mutex_lock(&buffer_mutex);
        while ((in + 1) % BUFFER_SIZE == out) {
            local_mutex_unlock(&buffer_mutex);
            local_mutex_lock(&mutex);
            local_mutex_unlock(&mutex);
            local_mutex_lock(&buffer_mutex);
            gettimeofday(&end, NULL);
            wait_time += (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
            gettimeofday(&start, NULL);
        }
        buffer[in] = item;
        printf("Producer produced item %d at index %d\n", item, in);
        in = (in + 1) % BUFFER_SIZE;
        local_mutex_unlock(&buffer_mutex);
        local_mutex_lock(&mutex);
        local_mutex_unlock(&mutex);
    }

    printf("Producer total wait time: %lld microseconds\n", wait_time);

    // 리소스 사용량 측정 (프로세스 전체)
    getrusage(RUSAGE_SELF, &usage);
    printf("Producer maximum resident set size: %ld kilobytes\n", usage.ru_maxrss);
    printf("Producer context switches: %ld\n", usage.ru_nvcsw + usage.ru_nivcsw);

    pthread_exit(NULL);
}

void *consumer(void *arg) {
    int item;
    struct timeval start, end;
    long long wait_time = 0;
    struct rusage usage;

    for (int i = 0; i < NUM_ITEMS; ++i) {
        gettimeofday(&start, NULL);
        local_mutex_lock(&buffer_mutex);
        while (in == out) {
            local_mutex_unlock(&buffer_mutex);
            local_mutex_lock(&mutex);
            local_mutex_unlock(&mutex);
            local_mutex_lock(&buffer_mutex);
            gettimeofday(&end, NULL);
            wait_time += (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
            gettimeofday(&start, NULL);
        }
        item = buffer[out];
        printf("Consumer consumed item %d from index %d\n", item, out);
        out = (out + 1) % BUFFER_SIZE;
        local_mutex_unlock(&buffer_mutex);
        local_mutex_lock(&mutex);
        local_mutex_unlock(&mutex);
    }

    printf("Consumer total wait time: %lld microseconds\n", wait_time);

    // 리소스 사용량 측정 (프로세스 전체)
    getrusage(RUSAGE_SELF, &usage);
    printf("Consumer maximum resident set size: %ld kilobytes\n", usage.ru_maxrss);
    printf("Consumer context switches: %ld\n", usage.ru_nvcsw + usage.ru_nivcsw);

    pthread_exit(NULL);
}

void test3_main() {
    pthread_t producer_thread, consumer_thread;
    struct rusage usage;

    // 뮤텍스 초기화
    local_mutex_init(&mutex);
    local_mutex_init(&buffer_mutex);

    // 스레드 생성
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // 스레드 종료 대기
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // 메인 스레드의 리소스 사용량 측정 (프로세스 전체)
    getrusage(RUSAGE_SELF, &usage);
    printf("Main thread maximum resident set size: %ld kilobytes\n", usage.ru_maxrss);
    printf("Main thread context switches: %ld\n", usage.ru_nvcsw + usage.ru_nivcsw);

    // 뮤텍스 제거
    local_mutex_destroy(&mutex);
    local_mutex_destroy(&buffer_mutex);

    //return 0;
}