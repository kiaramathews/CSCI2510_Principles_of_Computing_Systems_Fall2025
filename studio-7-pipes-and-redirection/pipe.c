#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int fd[2];
    pid_t child1, child2;

    if (pipe(fd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    child1 = fork();
    if (child1 < 0) {
        perror("fork failed");
        exit(1);
    } else if (child1 == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);  
        close(fd[1]);  

        char *cmd1 = "./program1";
        char *argv1[] = {"program1", NULL};
        execvp(cmd1, argv1);
        perror("Error exec'ing program1");
        exit(1);
    }

    child2 = fork();
    if (child2 < 0) {
        perror("fork failed");
        exit(1);
    } else if (child2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);  
        close(fd[0]);  

        char *cmd2 = "./program2";
        char *argv2[] = {"program2", NULL};
        execvp(cmd2, argv2);
        perror("Error exec'ing program2");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    printf("Parent: both children have finished.\n");
    return 0;
}
