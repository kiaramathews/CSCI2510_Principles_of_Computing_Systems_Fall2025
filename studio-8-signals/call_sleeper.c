//A simple program that fork()s and exec()s the ./sleep program

#include <unistd.h> //fork(), execvp(), perror(), waidpid() 
#include <stdlib.h> //For exit()
#include <stdio.h> //For printf()
#include <signal.h> // For signal

void ignore_sigint( int signum ){
    printf("\n[Caller] WARNING: Ignoring SIGINT (Signal number %d).\n", signum);
}

int main( int argc, char* argv[] ){

		pid_t ret;

	if (signal(SIGINT, ignore_sigint) == SIG_ERR) {
        perror("Could not set signal handler");
        exit(-1);
    }

		printf("Forking sleeper...\n");	

		ret = fork();
		if( ret == -1 ){
		perror("Could not fork");
		exit(-1);
		} 

		if( ret == 0 ){ //Child
			int exec_ret;
			char* cmd = "./sleep";
			char* myargv[] = {"sleep", NULL};
			exec_ret = execvp( cmd, myargv );
			if( exec_ret == -1 ){
				perror("Error calling execvp");
				exit(-1);
			}	

		}

	//Parent
	printf("Waiting for sleeper %d...\n", ret);
	waitpid( ret, NULL, 0 );

	printf("Parent finished waiting and returned successfully!\n");


	return 0;
}
