// local_mutex.c
// test.c
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "local_mutex.h"
// 뮤텍스 초기화 함수
int local_mutex_init(local_mutex_t *mutex) {
int ret;
// 재진입 가능한 뮤텍스 속성 설정
if (pthread_mutexattr_init(&mutex->attr) != 0) {
return errno;
}
if (pthread_mutexattr_settype(&mutex->attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
return errno;
}
// 뮤텍스 초기화
ret = pthread_mutex_init(&mutex->lock, &mutex->attr);
if (ret != 0) {
return ret;
}
return 0;
}
// 뮤텍스 잠금 함수
int local_mutex_lock(local_mutex_t *mutex) {
return pthread_mutex_lock(&mutex->lock);
}
// 뮤텍스 잠금 해제 함수
int local_mutex_unlock(local_mutex_t *mutex) {
return pthread_mutex_unlock(&mutex->lock);
}
// 뮤텍스 제거 함수
int local_mutex_destroy(local_mutex_t *mutex) {
int ret;
ret = pthread_mutex_destroy(&mutex->lock);
if (ret != 0) {
return ret;
}
ret = pthread_mutexattr_destroy(&mutex->attr);
if (ret != 0) {
return ret;
}
return 0;
}
// 타임아웃을 이용한 뮤텍스 잠금 시도 함수
int local_mutex_trylock_with_timeout(local_mutex_t *mutex, long timeout_ns) {
struct timespec ts;
clock_gettime(CLOCK_REALTIME, &ts);
// 현재 시간에 타임아웃 시간을 더한다.
ts.tv_nsec += timeout_ns;
while (ts.tv_nsec >= 1000000000L) {
ts.tv_nsec -= 1000000000L;
ts.tv_sec += 1;
}
return pthread_mutex_trylock(&mutex->lock);
}