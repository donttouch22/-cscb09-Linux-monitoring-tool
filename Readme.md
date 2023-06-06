# Concurrent Stats Functions
This program provides several functions for monitoring system usage and resources. The program consists of `stats_functions.h`, `stats_functions.c`, `a3.c`, and a `Makefile`.

## Approach

To make the code run in a concurrent fashion, I modified my A1 such that the `get` functions does not return the info found, but prints them out instead. 

In this modified version of the system monitoring tool, I created child processes using `fork()` to run the `get_curr_sys()`, `print_user_info()`, and `get_cpu_usage()` functions concurrently. We create pipes for communication with the child processes using `pipe()`, and then redirect the `stdout` of each child process to the corresponding pipe's write end using `dup2()`. In the parent process, we read the output of each child process from the respective pipe's read end using `read()` and print the results to the console. Finally, we close all the pipe file descriptors and wait for the child processes to complete using `wait()`.

## Contents of Files

### `stats_functions.h`

This file contains the declarations of functions that provide information about system resources. The functions are:

-   `long get_self_usage()`: This function returns the memory used by the current process.
-   `float get_curr_sys(int print)`: This function prints current system usage info and returns the current memory usage. The argument `print` is a boolean that denote whether a string on memory usage should be printed. When print == 1, the string is printed.
-   `void get_cpu_usage()`: This function calculates and prints total CPU usage.
-   `void section_break()`: This function prints 40 dashes.
-   `void print_self_usage()`: This function prints memory usage of the process `print_self_usage`.
-   `void print_user_info()`: This function prints logged users and their sessions.
-   `void print_cpu_info(char *buffer, int graphics, float cpu_usage)`: This function prints information about CPU utilization.
-   `void print_sys_info()`: This function prints system information.
-   `float print_mem_usage(char *buffer, char *buffer_1, float memory_used, float prev_memory_used, int sequential, int graphics)`: This function prints system information and returns the new memory usage.

### `stats_functions.c`

This file contains the implementation of the functions declared in `stats_functions.h`. The functions use system calls and other libraries to provide information about system resources.

### `a3.c`

This file contains the main program of the system monitoring tool running the functions defined in `stats_functions.c`.

- `void sigtstp_handler(int signum)`: This function acts as a signal handler for SIGTSTP (Ctrl-Z)
- `void sigintHandler(int signum)`: This function acts as a signal handler for Ctrl-C

### `Makefile`

This file contains the instructions to compile the program. The `Makefile` compiles the program using `gcc` and the flags `-Wall`. The object files are created by running `gcc` with the `-c` flag, and the program is linked using the `gcc` command. The `Makefile` also contains instructions to clean the program by removing the object files and the executable.

## Running the Program

To run the program, navigate to the program directory and run `make` to compile the code. Once compiled, run the `a3` executable. The program will prompt the user for a function to run. The program can be terminated by entering `Ctrl-C` .

```bash
make
./a3
```

By default, without any command line arguments, it would print out the metrics by refreshing the screen at a rate of 1 second by 10 times. The rate of sampling and number of samples can be cusstomized using command line arguments `--tdelay=T` and `--samples=N`, such that the result would be collected at N repetitions and sampled in the interval of T seconds. When no flags are present, The last two arguments can also be considered as _positional arguments_ if not flag is indicated in the corresponding order: `samples tdelay`.

To generate information concerning user usage only, you may use the flag `--user`. Similarly, the flag `--sytem` is used for generating system usage information only.