#ifndef _PIT_H
#define _PIT_H

#include "lib.h"
#include "types.h"
#include "drivers/drivers.h"
#include "x86_desc.h"
#include "pcb.h"

#define PIT_0_DATA 0x40
#define PIT_MODE_REG 0x43
#define PIT_DEFAULT_RATE 1193181
#define PIT_10_MS 100 // 1/.01
#define MAX_RUNNABLE_PROG 3
void pit_init();
void scheduling_init();
void schedule_handler();
void pit_irq_handler();

uint32_t runnable_tasks[MAX_RUNNABLE_PROG];//only 3 shells running and contains the PIDs




#endif

