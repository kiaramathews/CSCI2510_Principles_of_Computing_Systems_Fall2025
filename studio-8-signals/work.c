#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h> 

//work needs to be done
volatile sig_atomic_t work_to_do = 0;

// 1. Signal Handler for SIGUSR1
void handle_sigusr1( int signum ){
    printf("\n[Worker: %d] Received SIGUSR1. Setting flag to perform work.\n", getpid());
    work_to_do = 1; // Set the flag
}

int main(){
    printf("Worker's PID is: %d\n", getpid());
    
    // Register the custom handler for SIGUSR1
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR) {
        perror("Could not set SIGUSR1 handler");
        return 1;
    }
    
    printf("Worker is waiting for SIGUSR1...\n");
    
    while(1){
        if(work_to_do){
            // 2. Perform the useful work
            printf("[Worker: %d] --- PERFORMING ASYNCHRONOUS WORK! ---\n", getpid());
            sleep(2);
            printf("[Worker: %d] Work finished. Going back to waiting.\n", getpid());
            work_to_do = 0; 
        } else {
            pause(); 
        }
    }
    
    return 0;
}