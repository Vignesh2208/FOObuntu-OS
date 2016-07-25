#include "pit.h"
#include "pcb.h"
#include "systemcalls.h"
#include "paging.h"
#include "isr.h"
#include "i8259.h"
extern pcb * active_terminal_task;


pcb* head_runqueue;
//pcb* tail_runqueue;

/*
 * pit_init
 *   DESCRIPTION: Initialize the PIT called from kernel.c
 *   INPUTS: none
 *   OUTPUTS: None
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets PIT rate
 */ 
void pit_init(){
	//setup the pit for scheduling
	//Channel 0 
	//lobyte/hibyte
	//Mode 3 (standard for OSes)

	//set the mode register
	outb(0x36,PIT_MODE_REG);

	//set divider so rate is 10ms
	uint16_t divider = PIT_DEFAULT_RATE/PIT_10_MS;
	uint8_t lobyte,hibyte;
	lobyte=divider & 0xFF;
	hibyte=divider>>8;
	outb(lobyte,PIT_0_DATA);
	outb(hibyte,PIT_0_DATA);

	scheduling_init();
}

/*
 * pit_irq_handler
 *   DESCRIPTION: Handles scheduling/context switching and setup of PIT 
 				called from the handler from irq.c
 *   INPUTS: none
 *   OUTPUTS: None
 *   RETURN VALUE: none
 *   SIDE EFFECTS: schedules current processes running using round robin
 */ 
 void pit_irq_handler(){
 	
 	schedule_handler();

 	// printf("PIT %d\n",counter);
 }

/*
 * scheduling_init
 *   DESCRIPTION: Initializes the scheduling parameters called from kernel.c
 *   INPUTS: none
 *   OUTPUTS: None
 *   RETURN VALUE: none
 */ 
void scheduling_init(){
	head_runqueue = NULL;

	return;
}
/*
 * schedule_handler
 *   DESCRIPTION: Goes through the runqueue in the pcb (linked list)
 * 				in a round robin to schedule
 *   INPUTS: none
 *   OUTPUTS: None
 *   RETURN VALUE: none
 *   SIDE EFFECTS: shows only the active terminal 
 */ 
void schedule_handler(){

	if(head_runqueue == NULL)
		return;

	if(current->runqueue != NULL){
		next_task = current->runqueue;
	}
	else{
		next_task = head_runqueue;
	}
	load_paging(next_task->pid);
	// show_screen(active_terminal_task->curr_terminal);

}

