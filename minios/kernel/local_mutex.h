// local_mutex.h
typedef struct {
    pthread_mutex_t lock;
    pthread_mutexattr_t attr;
} local_mutex_t;

int local_mutex_init(local_mutex_t *mutex);
int local_mutex_lock(local_mutex_t *mutex);
int local_mutex_unlock(local_mutex_t *mutex);
int local_mutex_destroy(local_mutex_t *mutex);
int local_mutex_trylock_with_timeout(local_mutex_t *mutex, long timeout_ns);