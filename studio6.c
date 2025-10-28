#include <stdio.h>
#include <string.h> 
#include <unistd.h> 

#define BUFFER_SIZE 256

int main() {
    int max_args = 15; 
    int max_argv_size = max_args + 2; 
    char* cmd; 
    char* my_argv[max_argv_size];
    char input_string[BUFFER_SIZE];
    
    printf("Enter command: ");
    
    if (fgets(input_string, BUFFER_SIZE, stdin) == NULL) {
        return 1; 
    }

    char *newline_pos = strchr(input_string, '\n');
    if (newline_pos != NULL) {
        *newline_pos = '\0';
    }
    
    cmd = strtok(input_string, " "); 
    
    if (cmd == NULL) {
        return 0;
    }

    my_argv[0] = cmd;
    
    char *res;
    int i = 1; 
    
    while ((res = strtok(NULL, " ")) != NULL && i < max_argv_size - 1) {
        my_argv[i] = res;
        i++;
    }
    
    my_argv[i] = NULL; 

    execvp(cmd, my_argv);
    
    perror("execvp failed");
    return 1; 
}