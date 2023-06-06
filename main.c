#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <getopt.h>

#include "stats_functions.h"

#define MAX_BUFFER_SIZE 1024

// Signal handler for SIGTSTP (Ctrl-Z)
void sigtstp_handler(int signum) {
    printf("\nCtrl-Z signal received. Program cannot be run in background.\n");
}

// Signal handler for Ctrl-C
void sigintHandler(int signum) {
    char choice;
    printf("\nDo you want to quit? (y/n): ");
    fflush(stdout);
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y') {
        printf("Exiting...\n");
        exit(0);
    } else {
        printf("Resuming...\n");
    }
}

int main(int argc, char * const argv[])
{
	// Register signal handler for Ctrl-C
    signal(SIGINT, sigintHandler);

    // Register signal handler for SIGTSTP (Ctrl-Z)
    signal(SIGTSTP, sigtstp_handler);

	int N = 10, T = 1; /* N repetitions and frequency in T secs*/

	// parsing long options using getopt_long
	int c;
	int option_index = 0;
	int sys = 0, user = 0, graphics = 0, sequential = 0; /*flag variables*/

	// defines long_options structure for command line arguments
	struct option long_options[] = {
		{"system", no_argument, &sys, 1},
		{"user", no_argument, &user, 1},
		{"graphics", no_argument, &graphics, 1},
		{"sequential", no_argument, &sequential, 1},
		{"samples", optional_argument, NULL, 's'},
		{"tdelay", optional_argument, NULL, 't'},
		{0,0,0,0}
	};

	//handle positional arg
	if (argc >= 2 && atoi(argv[1])){
		N = atoi(argv[1]);
		if (argc >= 3 && atoi(argv[2])){
			T = atoi(argv[2]);
		}
	}

	while ((c = getopt_long(argc, argv, "s::t::", long_options, &option_index)) != -1){
		switch(c)
		{
		case 0:
			/* sets flag variables */
			break;
		case 's':
			if (optarg && atoi(optarg))
				N = atoi(optarg);
			break;
		case 't':
			if (optarg && atoi(optarg))
				T = atoi(optarg);
			break;
		case '?':
			break;
		default:
			/* invalid option */
			fprintf(stderr, "%c is invalid\n", optopt);
			return(1);
		}
	}

//////////////////////////////////////////////
// argument parsing done
//////////////////////////////////////////////

	// /* for calculating memory change in graphical representation */
	float prev_memory_used = get_curr_sys(0);

	/* prints out all stats as required */
	char buffer_1[1024] = ""; // storing the previous strings in non-sequential print
	char buffer_2[1024] = ""; // storing the previous strings in graphical cpu data
	
	for (int i = 0; i < N; ++i){

		int memory_pipe[2];
	    int users_pipe[2];
	    int cpu_pipe[2];
	    char buffer[MAX_BUFFER_SIZE];

	    // Create pipes for inter-process communication
	    if (pipe(memory_pipe) < 0 || pipe(users_pipe) < 0 || pipe(cpu_pipe) < 0) {
	        perror("Failed to create pipes");
	        exit(1);
	    }

	    // Fork to create child processes for each query
	    pid_t memory_pid = fork();
	    if (memory_pid < 0) {
	        perror("Failed to fork");
	        exit(1);
	    } else if (memory_pid == 0) {
	        // Child process for memory utilization
	        close(memory_pipe[0]); // Close read end of memory pipe
	        dup2(memory_pipe[1], STDOUT_FILENO); // Redirect stdout to memory pipe write end
			close(memory_pipe[1]); // close the write end of the pipe
			if (sys == 1 || (sys==0 && user==0))
				get_curr_sys(1);

	    	exit(0);
	    }

	    pid_t users_pid = fork();
	    if (users_pid < 0) {
	        perror("Failed to fork");
	        exit(1);
	    } else if (users_pid == 0) {
	        // Child process for connected users
	        close(users_pipe[0]); // Close read end of users pipe
	        dup2(users_pipe[1], STDOUT_FILENO); // Redirect stdout to users pipe write end
	        close(cpu_pipe[1]); // Close CPU pipe write end
	        if (user == 1 || (sys==0 && user==0))
	        	print_user_info();
	        exit(0);
	    }

	    pid_t cpu_pid = fork();
	    if (cpu_pid < 0) {
	        perror("Failed to fork");
	        exit(1);
	    } else if (cpu_pid == 0) {
	        // Child process for CPU utilization
	        close(cpu_pipe[0]); // Close read end of CPU pipe
	        dup2(cpu_pipe[1], STDOUT_FILENO); // Redirect stdout to CPU pipe write end
	        close(cpu_pipe[1]); // Close CPU pipe write end
	        
			// print cpu info
			if (sys == 1 || (sys==0 && user==0))
				get_cpu_usage();
	        exit(0);
	    }

	    // Parent process waits for child processes to finish
    	int status;
    	wait(&status);
    	wait(&status);
    	wait(&status);

	    // Parent process
	    close(memory_pipe[1]); // Close write end of memory pipe
	    close(users_pipe[1]); // Close write end of users pipe
	    close(cpu_pipe[1]); // Close write end of CPU pipe

	    if (sequential == 0){
			// non-sequential
			printf("\033[2J");   // Clear entire screen
    		printf("\033[0;0H"); // Move cursor to top-left corner
		}else{
			// sequential
			printf(">>> iteration %d\n", i+1);
		}

		/* print process self usage */
		print_self_usage(N,T);

	    // Read memory utilization from memory pipe
	    int memory_bytes_read = read(memory_pipe[0], buffer, MAX_BUFFER_SIZE);
	    if (memory_bytes_read > 0) {
	        buffer[memory_bytes_read] = '\0';/* buffer stores the string in memory usage for
									current iteration */

	        float memory_used;
	        sscanf(buffer, "%f", &memory_used);
			/* print system information */
	        print_mem_usage(buffer, buffer_1, memory_used, prev_memory_used, sequential, graphics);
			
			prev_memory_used = memory_used;
	    }

	    // Read connected users from users pipe
	    int users_bytes_read = read(users_pipe[0], buffer, MAX_BUFFER_SIZE);
	    if (users_bytes_read > 0) {
	        buffer[users_bytes_read] = '\0';
	        printf("%s", buffer);
	    }

	    // Read CPU utilization from CPU pipe
		int cpu_bytes_read = read(cpu_pipe[0], buffer, MAX_BUFFER_SIZE);
		if (cpu_bytes_read > 0) {
	    	buffer[cpu_bytes_read] = '\0';
	    	float cpu_usage;
	        sscanf(buffer, "%f", &cpu_usage);
	        print_cpu_info(buffer_2, graphics, cpu_usage);
		}

		// Close remaining pipe file descriptors
		close(memory_pipe[0]);
		close(users_pipe[0]);
		close(cpu_pipe[0]);

		sleep(T);
	}
	print_sys_info();
	return 0;
}