#ifndef _SYSTEMCALLS_H
#define _SYSTEMCALLS_H

#include "lib.h"
#include "types.h"
#include "drivers/drivers.h"
#include "x86_desc.h"
#include "pcb.h"


#define FOUR_MB 0x400000
#define KERNEL_STACK_SIZE 0x2000
#define IMAGE_OFF 0x00048000
#define IMAGE_ADDR 0x08000000//program will execute at 0x8048000
#define PID_OFF 0x2000 //8KB all PID will be inside kernel
#define PROGRAM_LOAD_START_ADDRESS 0xC00000
#define INTERRUPT_ENABLE_FLAG_BIT 9
#define UNUSED_PID 32000


// SYS_CALL_NUMBERS
#define HALT 1
#define EXECUTE 2
#define READ 3
#define WRITE 4
#define OPEN 5
#define CLOSE 6
#define GETARGS 7
#define VIDMAP 8
#define SETHANDLER 9
#define SIGRETURN 10


pid_t used_pids[MAX_N_PROCESSES];
terminal_t terminals[MAX_N_PROCESSES];


int execute(const uint8_t* command);

int32_t open(const uint8_t * filename);
int32_t write(uint32_t fd, uint8_t * buf, int size);
int32_t read(uint32_t fd, uint8_t * buf, int size);
int32_t close(uint32_t fd);
int32_t halt(uint8_t status);

int32_t getargs(uint8_t * buf, int32_t nbytes);
int32_t vidmap(uint8_t **screen_start);

#endif
