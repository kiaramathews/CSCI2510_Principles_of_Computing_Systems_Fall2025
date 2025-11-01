#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

// Helper function to handle the fork and execvp logic
void execute_program(char* path, char* arg) {
    char* cmd = path;
    // execvp requires an array of arguments, terminated by NULL
    char* myargv[] = {arg, NULL};
    
    // execvp replaces the current process image with the new program
    int ret = execvp(cmd, myargv);
    
    // If execvp returns, it means an error occurred
    if (ret == -1) {
        perror("Error exec'ing program");
        exit(EXIT_FAILURE);
    }
}

int main() {
    pid_t pid1, pid2;

    printf("Parent process (PID: %d) starting...\n", getpid());

    // --- Fork for Program 1 (Writer: ./print) ---
    pid1 = fork();
    if (pid1 == 0) {
        // Child 1: Execute Program 1 (./print)
        execute_program("./print", "print");
    } else if (pid1 < 0) {
        perror("Fork 1 failed");
        exit(EXIT_FAILURE);
    }

    // --- Fork for Program 2 (Reader: ./readloop) ---
    pid2 = fork();
    if (pid2 == 0) {
        // Child 2: Execute Program 2 (./readloop)
        execute_program("./readloop", "readloop");
    } else if (pid2 < 0) {
        perror("Fork 2 failed");
        exit(EXIT_FAILURE);
    }
    
    // --- Parent Process ---
    // Wait for both children to terminate
    if (pid1 > 0 && pid2 > 0) {
        printf("Parent: Waiting for children...\n");
        
        // Child 1 (./print) finishes quickly
        waitpid(pid1, NULL, 0); 
        
        // Child 2 (./readloop) will hang waiting for input
        waitpid(pid2, NULL, 0); 
        
        printf("Parent: Both children terminated. Exiting.\n");
    }

    return 0;
}