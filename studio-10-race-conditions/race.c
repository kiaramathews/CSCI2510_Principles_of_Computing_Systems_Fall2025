#include <stdio.h>
#include <pthread.h>

#define ITERATIONS 2000000  


int race = 0;


void* adder(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        race++;   
    }
    return NULL;
}

void* subtractor(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        race--;   
    }
    return NULL;
}

// main function 
int main() {
    pthread_t t1, t2;

    race = 0; 

    pthread_create(&t1, NULL, adder, NULL);
    pthread_create(&t2, NULL, subtractor, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Final race value: %d\n", race);
    return 0;
}
