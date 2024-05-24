#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>

#include "semaphore.h"

semaphore_t semaphoreWater_sema;
pthread_cond_t condWater_sema;
int water_sema = 0;
struct timespec start_time_sema, end_time_sema;
double total_wait_time_sema = 0;
double total_response_time_sema = 0;
int total_operations_sema = 0;

void *purifierThread_sema(void *arg) {
    for (int i = 0; i < 5; i++) {
        struct timespec wait_start, wait_end;
        clock_gettime(CLOCK_REALTIME, &wait_start);
        semaphore_wait(&semaphoreWater_sema);
        clock_gettime(CLOCK_REALTIME, &wait_end);
        double wait_time = (wait_end.tv_sec - wait_start.tv_sec) + (wait_end.tv_nsec - wait_start.tv_nsec) / 1e9;
        total_wait_time_sema += wait_time;
        water_sema += 70;
        printf("============================물 채움. 현재 물의 양: %d\n", water_sema);
        pthread_cond_signal(&condWater_sema);
        semaphore_signal(&semaphoreWater_sema);
        sleep(1);
    }
    return NULL;
}

void *drinkerThread_sema(void *arg) {
    struct timespec wait_start, wait_end, response_start, response_end;
    clock_gettime(CLOCK_REALTIME, &response_start);
    clock_gettime(CLOCK_REALTIME, &wait_start);
    semaphore_wait(&semaphoreWater_sema);
    clock_gettime(CLOCK_REALTIME, &wait_end);
    double wait_time = (wait_end.tv_sec - wait_start.tv_sec) + (wait_end.tv_nsec - wait_start.tv_nsec) / 1e9;
    total_wait_time_sema += wait_time;
    while (water_sema < 40) {
        printf("물이 부족하니 기다릴게.. \n");
        pthread_cond_wait(&condWater_sema, &semaphoreWater_sema.mutex);
    }
    water_sema -= 40;
    printf("고마워. 물을 마시고 나니 남은 양: %d\n", water_sema);
    semaphore_signal(&semaphoreWater_sema);
    clock_gettime(CLOCK_REALTIME, &response_end);
    double response_time = (response_end.tv_sec - response_start.tv_sec) + (response_end.tv_nsec - response_start.tv_nsec) / 1e9;
    total_response_time_sema += response_time;
    total_operations_sema++;
    return NULL;
}

void measure_performance_sema() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("User CPU time used: %ld.%06d sec\n", usage.ru_utime.tv_sec, (int)usage.ru_utime.tv_usec);
    printf("System CPU time used: %ld.%06d sec\n", usage.ru_stime.tv_sec, (int)usage.ru_stime.tv_usec);
    printf("Maximum resident set size: %ld KB\n", usage.ru_maxrss);
    printf("Context switches: voluntary: %ld, involuntary: %ld\n", usage.ru_nvcsw, usage.ru_nivcsw);
    printf("Average wait time: %.6f sec\n", total_wait_time_sema / total_operations_sema);
    printf("Average response time: %.6f sec\n", total_response_time_sema / total_operations_sema);
}

void test2_sema_main(int argc, char *argv[]) {
    int numThreads;
    printf("생성할 스레드 수를 입력하세요: ");
    scanf("%d", &numThreads);
    pthread_t *th = (pthread_t *)malloc(numThreads * sizeof(pthread_t));

    semaphore_init(&semaphoreWater_sema, 1);
    pthread_cond_init(&condWater_sema, NULL);

    int numDrinkers = numThreads / 3 * 2;
    int numPurifiers = numThreads - numDrinkers;

    // Initial water level to prevent initial deadlock
    water_sema = 1000;

    clock_gettime(CLOCK_REALTIME, &start_time_sema);

    for (int i = 0; i < numThreads; i++) {
        if (i < numDrinkers) {
            if (pthread_create(&th[i], NULL, &drinkerThread_sema, NULL) != 0) {
                perror("failed to create thread");
            }
        } else {
            if (pthread_create(&th[i], NULL, &purifierThread_sema, NULL) != 0) {
                perror("Failed to create thread");
            }
        }
    }

    for (int i = 0; i < numThreads; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }

    clock_gettime(CLOCK_REALTIME, &end_time_sema);
    double elapsed_time = (end_time_sema.tv_sec - start_time_sema.tv_sec) + (end_time_sema.tv_nsec - start_time_sema.tv_nsec) / 1e9;
    printf("Total elapsed time: %.6f sec\n", elapsed_time);

    measure_performance_sema();

    semaphore_destroy(&semaphoreWater_sema);
    pthread_cond_destroy(&condWater_sema);
    free(th);
}
