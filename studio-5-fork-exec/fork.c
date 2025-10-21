#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        printf("Fork failed.\n");
        return 1;
    } else if (pid == 0) {
        char command[50];
        printf("Enter a command to run (e.g., ls, date, whoami): ");
        scanf("%49s", command);

        char* child_argv[] = {command, NULL};

        execvp(child_argv[0], child_argv);

        printf("Error: exec failed for %s\n", command);
    } else {
        waitpid(pid, NULL, 0);
        printf("Parent process: PID = %d\n", getpid());
    }
    return 0;
}
