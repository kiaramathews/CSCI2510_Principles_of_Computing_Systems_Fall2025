#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_LINE 256
#define MAX_ARGS 15
#define MAX_CMDS 50

volatile sig_atomic_t interrupted = 0;  //for signal handling

// signal handler for SIGINT
void sigint_handler(int sig) { 
    interrupted = 1;
    write(STDOUT_FILENO, "\n", 1);
}

typedef struct {
    char *args[MAX_ARGS + 1];  //1 is for null terminator
    int arg_count;
} Command;

//extra credit 
void print_prompt() {   //prints the prompt along with the current directory
    char cwd[MAX_LINE];
    char *home = getenv("HOME");
    
    if (getcwd(cwd, sizeof(cwd)) != NULL && home != NULL) {
        int home_len = strlen(home);
        
        // checks if the current directory is in the home directory
        if (strncmp(cwd, home, home_len) == 0) { //current path
            if (cwd[home_len] == '\0') { //path equals home
                printf("slush|~> ");
            } else if (cwd[home_len] == '/') { //path equals home/subdirectory
                printf("slush|~%s> ", &cwd[home_len]);
            } else {
                // Path starts with home but isn't a subdirectory
                printf("slush|%s> ", cwd);
            }
        } else {
            // outside the home directory
            printf("slush|%s> ", cwd);
        }
    } else {
        printf("slush> ");
    }
    
    fflush(stdout);
}

// parses the input lines into commands separated by '('
//return the number of commands parsed
int parse_input(char *line, Command *commands) {
    char *token;   //pointer to current
    int cmd_count = 0; //num of commands found
    int arg_index = 0;  //current arguement
    
    line[strcspn(line, "\n")] = 0;
    
    token = strtok(line, " \t");
    
    if (token == NULL) {
        return 0;  //if not tokens are found returns an empty line
    }
    
    commands[cmd_count].arg_count = 0;
    
    while (token != NULL) {
        if (strcmp(token, "(") == 0) {
            if (commands[cmd_count].arg_count == 0) {
                fprintf(stderr, "Invalid null command\n");
                return -1;
            }
            commands[cmd_count].args[commands[cmd_count].arg_count] = NULL;
            cmd_count++;
            
            if (cmd_count >= MAX_CMDS) {
                fprintf(stderr, "Too many commands in pipeline\n");
                return -1;
            }
            
            commands[cmd_count].arg_count = 0;
            arg_index = 0;
        } else {
            // adds an argument to the current command
            if (arg_index >= MAX_ARGS) {
                fprintf(stderr, "Too many arguments\n");
                return -1;
            }
            commands[cmd_count].args[arg_index] = token;
            commands[cmd_count].arg_count++;
            arg_index++;
        }
        
        token = strtok(NULL, " \t");
    }
    
    if (commands[cmd_count].arg_count == 0 && cmd_count > 0) {
        fprintf(stderr, "Invalid null command\n");
        return -1;
    }
    
    if (commands[cmd_count].arg_count > 0) {
        commands[cmd_count].args[commands[cmd_count].arg_count] = NULL;
        cmd_count++;
    }
    
    return cmd_count;
}

void execute_command(Command *cmd, int input_fd) {
    if (input_fd != STDIN_FILENO) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    
    execvp(cmd->args[0], cmd->args);
    
    perror(cmd->args[0]);
    exit(1);
}

int execute_pipeline(Command *commands, int cmd_index, int num_cmds, int input_fd) {
    if (cmd_index == num_cmds - 1) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            return -1;
        }
        
        if (pid == 0) {
            execute_command(&commands[cmd_index], input_fd);
        }
        
        if (input_fd != STDIN_FILENO) {
            close(input_fd);
        }
        
        return pid;
    }
    
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return -1;
    }
    
    int child_pid = execute_pipeline(commands, cmd_index + 1, num_cmds, pipefd[0]);
    
    if (child_pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    
    if (pid == 0) {
        close(pipefd[0]);  
        
        // redirects the stdout to pipe write end
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        
        execvp(commands[cmd_index].args[0], commands[cmd_index].args);
        
        perror(commands[cmd_index].args[0]);
        exit(1);
    }
    
    close(pipefd[0]);
    close(pipefd[1]);
    
    if (input_fd != STDIN_FILENO) {
        close(input_fd);
    }
    
    return pid;
}

// waits for all the child processes
void wait_for_children(int num_children) {
    int status;
    for (int i = 0; i < num_children; i++) {
        while (wait(&status) < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
    }
}

int main() {
    char line[MAX_LINE];
    Command commands[MAX_CMDS];
    struct sigaction sa;
    
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    
    print_prompt();
    
    while (1) {
        interrupted = 0;
        
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }
        
        if (interrupted) {
            print_prompt();
            continue;
        }
        
        int num_cmds = parse_input(line, commands);
        
        if (num_cmds < 0) {
            print_prompt();
            continue;
        }
        
        if (num_cmds == 0) {
            print_prompt();
            continue;
        }
        
        if (strcmp(commands[0].args[0], "cd") == 0) {
            if (commands[0].arg_count < 2) {
                fprintf(stderr, "cd: missing argument\n");
            } else if (num_cmds > 1) {
                fprintf(stderr, "cd: must appear alone\n");
            } else {
                if (chdir(commands[0].args[1]) < 0) {
                    perror("cd");
                }
            }
            print_prompt();
            continue;
        }
        
        // executes the pipeline
        int first_pid = execute_pipeline(commands, 0, num_cmds, STDIN_FILENO);
        
        if (first_pid > 0) {
            wait_for_children(num_cmds);
        }
        
        print_prompt();
    }
    
    return 0;
}