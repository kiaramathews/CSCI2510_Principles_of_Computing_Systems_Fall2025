#include <stdio.h>
#include <pthread.h>

#define ITERS 20000000L

int race = 0;

typedef struct {
    pthread_mutex_t *mutex;
} thread_args_t;

void *adder(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;

    // makes it lock only once per thread
    pthread_mutex_lock(args->mutex);

    for (long i = 0; i < ITERS; i++) {
        race++;
    }

    // unlocks after 
    pthread_mutex_unlock(args->mutex);

    return NULL;
}

void *subtractor(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;

    // makes it lock only once per thread
    pthread_mutex_lock(args->mutex);

    for (long i = 0; i < ITERS; i++) {
        race--;
    }

    // unlocks
    pthread_mutex_unlock(args->mutex);

    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    pthread_mutex_t race_mutex;
    thread_args_t args1, args2;

    pthread_mutex_init(&race_mutex, NULL);

    args1.mutex = &race_mutex;
    args2.mutex = &race_mutex;

    pthread_create(&t1, NULL, adder, &args1);
    pthread_create(&t2, NULL, subtractor, &args2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Final race value: %d\n", race);

    pthread_mutex_destroy(&race_mutex);
    return 0;
}
