#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <utmp.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include "stats_functions.h"

long get_self_usage(){
	/* get the memory used by the current process */
	struct rusage r_usage;
	getrusage(RUSAGE_SELF,&r_usage);
	return r_usage.ru_maxrss;
}

float get_curr_sys(int print){
	/* prints current system usage info 
	print is a boolean that denotes whether a string should be printed
	returns current memory usage*/
    struct sysinfo info;
	const double gb = 1024*1024*1024;
	if (sysinfo(&info) == 0){
		double total_phy_ram = (double)info.totalram * (double) info.mem_unit;
		double total_vir_ram = ((double)info.totalram+(double)info.totalswap) * (double)info.mem_unit;
		double free_phy_ram = (double)info.freeram * (double)info.mem_unit;
		double free_vir_ram = ((double)info.freeswap+(double)info.freeram) * (double)info.mem_unit;
		if (print == 1)
			printf("%.2f GB / %.2f GB  --  %.2f GB / %.2f GB",(total_phy_ram-free_phy_ram)/gb ,total_phy_ram/gb,(total_vir_ram-free_vir_ram)/gb, total_vir_ram/gb);
		return (float)(total_phy_ram-free_phy_ram)/gb;
	}
	return -1;
}

void get_cpu_usage(){
	/* calculate and print total cpu usage */
    FILE* stat_file = fopen("/proc/stat", "r");
    if (stat_file == NULL) {
        perror("Failed to open /proc/stat");
        return;
    }

    // Variables to store CPU time values
    unsigned long long t1_user, t1_nice, t1_sys, t1_idle, t1_iowait, t1_irq, t1_softirq;
    unsigned long long t2_user, t2_nice, t2_sys, t2_idle, t2_iowait, t2_irq, t2_softirq;

    // Read the first set of CPU time values
    fscanf(stat_file, "cpu %llu %llu %llu %llu %llu %llu %llu",
           &t1_user, &t1_nice, &t1_sys, &t1_idle, &t1_iowait, &t1_irq, &t1_softirq);
    fclose(stat_file);

    // Delay for 10000 ms to get the second set of CPU time values
    usleep(10000);

    // Open /proc/stat file again for reading
    stat_file = fopen("/proc/stat", "r");
    if (stat_file == NULL) {
        perror("Failed to open /proc/stat");
        return;
    }

    // Read the second set of CPU time values
    fscanf(stat_file, "cpu %llu %llu %llu %llu %llu %llu %llu",
           &t2_user, &t2_nice, &t2_sys, &t2_idle, &t2_iowait, &t2_irq, &t2_softirq);
    fclose(stat_file);

    // Calculate CPU utilization
    unsigned long long t1_total = t1_user + t1_nice + t1_sys + t1_idle + t1_iowait + t1_irq + t1_softirq;
    unsigned long long t2_total = t2_user + t2_nice + t2_sys + t2_idle + t2_iowait + t2_irq + t2_softirq;
    unsigned long long t1_idle_time = t1_idle + t1_iowait;
    unsigned long long t2_idle_time = t2_idle + t2_iowait;
    double cpu_usage = ((double)(t2_total - t1_total - (t2_idle_time - t1_idle_time)) / (t2_total - t1_total)) * 100.0;

    printf("%.2f\n", cpu_usage);
}

void section_break(){
	/* prints 40 dashes*/
	for (int i = 0; i < 40; ++i)
	{
		putchar('-');
	}
	putchar('\n');
}

void print_self_usage(int N, int T){
	/* prints memory usage of the process print_self_usage	*/
	printf("Nbr of samples: %d -- every %d secs\n", N, T);
		long j = get_self_usage();
		printf("Memory usage: %ld kilobytes\n", j);
		section_break();
}

void print_user_info(){
	/* prints loged users and their sessions */
	printf("### Sessions/users ###\n");
	struct utmp *entry;
	setutent();
	entry = getutent();
	while(entry){
		if(entry->ut_type==USER_PROCESS) {
      		printf("%s", entry->ut_user);
      		printf(" %s (%s)\n", entry->ut_line, entry->ut_host);
    	}
    	entry=getutent();
	}
	endutent();
	section_break();
}

void print_cpu_info(char *buffer, int graphics, float cpu_usage){
	/* prints info about cpu utilization */
	const char divider = '|';

	long num_processors = sysconf(_SC_NPROCESSORS_ONLN);
	printf("Number of cores: %d\n", (int)num_processors);
	printf("total cpu use = %.2f%% \n", cpu_usage);
	// graphical representation
	if (graphics == 1){
		printf("%s", buffer);
		int mem_usage_width = (int)(cpu_usage);// Width of memory usage graph
		if (mem_usage_width>0){
			// positive change
			for (int i = 0; i < mem_usage_width; i++) {
            printf("%c", divider);
            strncat(buffer, &divider, 1);
        	}
		}
		char buffer_2 [10];
		sprintf(buffer_2, "%.2f \n",cpu_usage);
		printf("%s\n", buffer_2);
		strcat(buffer, buffer_2);
	}
	section_break();
}

void print_sys_info(){
	/* prints system information */
	struct utsname uts;
	if (uname(&uts) == 0){
		printf("### System Information ###\n");
		printf("System Name  = %s\n", uts.sysname);
		printf("Machine Name = %s\n", uts.nodename);
		printf("Version = %s\n", uts.version);
		printf("Architecture = %s\n", uts.machine);
	}
	section_break();
}

void print_mem_usage(char *buffer, char *buffer_1, float memory_used, float prev_memory_used, int sequential, int graphics){
	/* print system information */
	/* buffer is string on current memory usage , buffrer_1 contains all previous strings printed*/
	// do constants count as global variables? I put them here just to be safe
	const char newline = '\n';
	const char hash = '#';
	const char colon = ':';
	const char asterisk = '*';
	const char at = '@';
	const char divider = '|';
	const char space = ' ';

	printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");

	/* calculating memory change for graphical representation */
	float memory_change = memory_used - prev_memory_used;

	/* graphics */
	strcat(buffer_1, buffer);	/* strcat the current string with previous
								strings for printing non-sequentially */
	if (sequential == 0){
		/* if printing non-sequentially, include previous strings */
		printf("%s",buffer_1);
	}else{
		/* prints info on this iteration only */
		printf("%s", buffer);
	}

	if (graphics == 1){
		// print graphic representation
		printf("| ");
		strncat(buffer_1, &divider, 1);	// storing the string for non-sequential print
		strncat(buffer_1, &space, 1);

		int max_width = 20;	// max width of the graph
		int mem_usage_width = (int)(memory_change * max_width / 100.0);// Width of memory usage graph
		
		if (mem_usage_width>0){
			// positive change
			for (int j = 0; j < mem_usage_width; j++) {
            printf("%c", hash);
            strncat(buffer_1, &hash, 1);
        	}
        	printf("%c ", asterisk);
        	strncat(buffer_1, &asterisk, 1);
        	strncat(buffer_1, &space, 1);
		}else if(mem_usage_width>0){
			// negative change
			for (int j = mem_usage_width; j > 0; j--) {
            printf("%c", colon);
            strncat(buffer_1, &colon, 1);
        	}
        	printf("%c ", at);
        	strncat(buffer_1, &at, 1);
        	strncat(buffer_1, &space, 1);
    	}
    	// prints memory change at the end
    	char str_mem_change[20];
		sprintf(str_mem_change, "%.2f (%.2f)\n", memory_change, memory_used);
		printf("%s", str_mem_change);
		strcat(buffer_1, str_mem_change);
	}else{
		// no graphical representation
		printf("\n");
		strncat(buffer_1, &newline, 1);
	}
	section_break();
}