#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>

#include "local_mutex.h"

local_mutex_t mutexWater;
pthread_cond_t condWater;
int water = 0;
struct timespec start_time, end_time;
double total_wait_time = 0;
double total_response_time = 0;
int total_operations = 0;

void *purifier(void *arg) {
    for (int i = 0; i < 5; i++) {
        struct timespec wait_start, wait_end;
        clock_gettime(CLOCK_REALTIME, &wait_start);
        local_mutex_lock(&mutexWater);
        clock_gettime(CLOCK_REALTIME, &wait_end);
        double wait_time = (wait_end.tv_sec - wait_start.tv_sec) + (wait_end.tv_nsec - wait_start.tv_nsec) / 1e9;
        total_wait_time += wait_time;
        water += 70;
        printf("============================물 채움. 현재 물의 양: %d\n", water);
        local_mutex_unlock(&mutexWater);
        pthread_cond_signal(&condWater);
        sleep(1);
    }
    return NULL;
}

void *drink(void *arg) {
    struct timespec wait_start, wait_end, response_start, response_end;
    clock_gettime(CLOCK_REALTIME, &response_start);
    clock_gettime(CLOCK_REALTIME, &wait_start);
    local_mutex_lock(&mutexWater);
    clock_gettime(CLOCK_REALTIME, &wait_end);
    double wait_time = (wait_end.tv_sec - wait_start.tv_sec) + (wait_end.tv_nsec - wait_start.tv_nsec) / 1e9;
    total_wait_time += wait_time;
    while (water < 40) {
        printf("물이 부족하니 기다릴게.. \n");
        pthread_cond_wait(&condWater, &mutexWater.lock);
    }
    water -= 40;
    printf("고마워. 물을 마시고 나니 남은 양: %d\n", water);
    local_mutex_unlock(&mutexWater);
    clock_gettime(CLOCK_REALTIME, &response_end);
    double response_time = (response_end.tv_sec - response_start.tv_sec) + (response_end.tv_nsec - response_start.tv_nsec) / 1e9;
    total_response_time += response_time;
    total_operations++;
    return NULL;
}

void measure_performance() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("User CPU time used: %ld.%06d sec\n", usage.ru_utime.tv_sec, (int)usage.ru_utime.tv_usec);
    printf("System CPU time used: %ld.%06d sec\n", usage.ru_stime.tv_sec, (int)usage.ru_stime.tv_usec);
    printf("Maximum resident set size: %ld KB\n", usage.ru_maxrss);
    printf("Context switches: voluntary: %ld, involuntary: %ld\n", usage.ru_nvcsw, usage.ru_nivcsw);
    printf("Average wait time: %.6f sec\n", total_wait_time / total_operations);
    printf("Average response time: %.6f sec\n", total_response_time / total_operations);
}

void test2_main(int argc, char *argv[]) {
    int numThreads;
    printf("생성할 스레드 수를 입력하세요: ");
    scanf("%d", &numThreads);
    pthread_t *th = (pthread_t *)malloc(numThreads * sizeof(pthread_t));

    // 로컬 뮤텍스 초기화
    if (local_mutex_init(&mutexWater) != 0) {
        fprintf(stderr, "Mutex initialization failed.\n");
        //return 1;
    }

    pthread_cond_init(&condWater, NULL);

    int numDrinkers = numThreads / 3 * 2;
    int numPurifiers = numThreads - numDrinkers;

    clock_gettime(CLOCK_REALTIME, &start_time);

    for (int i = 0; i < numThreads; i++) {
        if (i < numDrinkers) {
            if (pthread_create(&th[i], NULL, &drink, NULL) != 0) {
                perror("failed to create thread");
            }
        } else {
            if (pthread_create(&th[i], NULL, &purifier, NULL) != 0) {
                perror("Failed to create thread");
            }
        }
    }

    for (int i = 0; i < numThreads; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }

    clock_gettime(CLOCK_REALTIME, &end_time);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("Total elapsed time: %.6f sec\n", elapsed_time);

    measure_performance();

    local_mutex_destroy(&mutexWater);
    pthread_cond_destroy(&condWater);
    free(th);

    //return 0;
}