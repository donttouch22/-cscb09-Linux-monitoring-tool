#ifndef STATS_FUNCTIONS_H
#define STATS_FUNCTIONS_H

/*	get the memory used by the current process	*/
	long get_self_usage();

/* prints current system usage info; returns current memory usage*/
	float get_curr_sys(int print);

/* calculate and print total cpu usage */
	void get_cpu_usage();

/*	prints 40 dashes	*/
	void section_break();

/*	prints memory usage of the process print_self_usage	*/
	void print_self_usage();

/*	prints looged users and their sessions */
	void print_user_info();

/*	prints info about cpu utilization */
	void print_cpu_info(char *buffer, int graphics, float cpu_usage);

/*	prints system information */
	void print_sys_info();

/*	print system information
	return new memory usage */
	void print_mem_usage(char *buffer, char *buffer_1, float memory_used, float prev_memory_used, int sequential, int graphics);

#endif
