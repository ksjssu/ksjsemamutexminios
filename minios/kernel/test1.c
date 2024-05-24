#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/times.h>
#include "local_mutex.h"

#define NUM_THREADS 5
#define LOG_FILE "log.txt"

local_mutex_t log_mutex;

void print_thread_cpu_mem_usage(pthread_t thread) {
    (void)thread; // 사용하지 않는 변수에 대한 경고를 피하기 위해
    printf("Thread CPU and memory usage information is not available on this platform.\n");
}
/*void print_thread_cpu_mem_usage(pthread_t thread) {
    kern_return_t kr;
    thread_basic_info_data_t thread_info;
    mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
    kr = thread_info(pthread_mach_thread_np(thread), THREAD_BASIC_INFO, (thread_info_t)&thread_info, &count);
    if (kr == KERN_SUCCESS) {
        double cpu_time = (double)thread_info.user_time.seconds + (double)thread_info.user_time.microseconds / 1000000.0;
        printf("Thread %p CPU time: %.6f seconds\n", (void *)thread, cpu_time);
        printf("Thread %p Memory usage: %zu bytes\n", (void *)thread, (size_t)thread_info.resident_size);
    } else {
        printf("Failed to get thread information: %s\n", mach_error_string(kr));
    }
}*/

void *thread_func(void *arg) {
    int thread_id = *(int *)arg;
    free(arg);
    pthread_t thread = pthread_self();

    struct tms tms_buffer;
    clock_t start_time, end_time;

    for (int i = 0; i < 10; i++) {
        start_time = times(&tms_buffer);

        // 뮤텍스 잠금
        if (local_mutex_lock(&log_mutex) != 0) {
            fprintf(stderr, "Mutex lock failed.\n");
            return NULL;
        }

        // 로그 파일에 메시지 작성
        FILE *fp = fopen(LOG_FILE, "a");
        fprintf(fp, "Thread %d: Message %d\n", thread_id, i);
        fclose(fp);

        // 뮤텍스 해제
        if (local_mutex_unlock(&log_mutex) != 0) {
            fprintf(stderr, "Mutex unlock failed.\n");
            return NULL;
        }

        end_time = times(&tms_buffer);
        double real_time = (double)(end_time - start_time) / sysconf(_SC_CLK_TCK);
        double user_time = (double)tms_buffer.tms_utime / sysconf(_SC_CLK_TCK);
        double system_time = (double)tms_buffer.tms_stime / sysconf(_SC_CLK_TCK);

        printf("Thread %p, Iteration %d:\n", (void *)thread, i);
        printf("Real time (response time): %.6f seconds\n", real_time);
        printf("User time: %.6f seconds\n", user_time);
        printf("System time (wait time): %.6f seconds\n", system_time);

        print_thread_cpu_mem_usage(thread); // 스레드 CPU 시간과 메모리 사용량 출력
        sleep(1); // 1초 대기
    }

    return NULL;
}

void test1_main() {
    pthread_t threads[NUM_THREADS];

    // 뮤텍스 초기화
    if (local_mutex_init(&log_mutex) != 0) {
        fprintf(stderr, "Mutex initialization failed.\n");
        //return 1;
    }

    // 스레드 생성
    for (int i = 0; i < NUM_THREADS; i++) {
        int *thread_id = malloc(sizeof(int));
        *thread_id = i;
        pthread_create(&threads[i], NULL, thread_func, thread_id);
    }

    // 스레드 종료 대기
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 뮤텍스 제거
    local_mutex_destroy(&log_mutex);

}