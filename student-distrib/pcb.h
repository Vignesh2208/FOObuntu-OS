#ifndef PCB_H
#define PCB_H
#include "x86_desc.h"

#define MAX_N_PROCESSES 6
#define MAX_N_FILES 6
#define MAX_ARG_SIZE 64
#define MAX_CMD_SIZE 64
typedef unsigned int pid_t;

typedef int32_t (*func_close)(int32_t fd);
typedef int32_t (*func_read)(uint32_t fd, uint8_t *buf, int size);
typedef int32_t (*func_write)(uint32_t fd, uint8_t *buf, int size);

typedef struct fd_struct{
	func_close fops_close;
	func_read fops_read;
	func_write fops_write;
	uint32_t offset;
	int32_t inode;
	uint8_t *flags;
	uint8_t is_used;
}fd;

typedef struct pcb_t{
	
	tss_t hardware_context;
	tss_t secondary_hardware_context;
	int type_of_return;
	// terminal_t * curr_terminal;
	void * curr_terminal;
	struct pcb_t * parent;
	struct pcb_t * child;
	
	struct pcb_t * runqueue; //the next pcb in the queue
	
	//parent stuff
	pid_t pid;
	uint8_t is_shell; //flag to check if current process is a shell
	uint8_t args[MAX_ARG_SIZE];//max could be set otherwise

	// File descriptor array
	uint32_t num_files_open;
	fd fd_array[MAX_N_FILES];

}pcb;

//Macro to get the current process pcb
extern pcb * get_current();

pcb * next_task;

#define current get_current()

#endif
