#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5

struct thread_data {
    int arg1;
    char arg2[100];
    int ret;
};

void* thread_entry(void* args) {
    struct thread_data* arg_ptr = (struct thread_data*) args;
    printf("Thread %d started. Message: %s\n", arg_ptr->arg1, arg_ptr->arg2);
    arg_ptr->ret = arg_ptr->arg1 * 10;
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    struct thread_data args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].arg1 = i;
        snprintf(args[i].arg2, sizeof(args[i].arg2), "This is thread %d", i);
        pthread_create(&threads[i], NULL, thread_entry, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %d returned %d\n", i, args[i].ret);
    }

    printf("All threads finished.\n");
    return 0;
}
