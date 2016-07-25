#include "isr.h"
#include "x86_desc.h"
#include "i8259.h"
#include "lib.h"
#include "systemcalls.h"
#include "pit.h"
#include "paging.h"


#define BITMASK_HALT_8 0xff
#define BITMASK_HALT_16 0xffff
extern int read_cr2();

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, user_esp, ss;   /* pushed by the processor automatically */ 
};
/*
 * handle_div_exception
 *   DESCRIPTION: Jump table entry from exception_handler and prints division by 0 exception
 *   INPUTS: none
 *   OUTPUTS: Prints Div by zero onto screen
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clears screen  
 */  
void handle_div_exception(){

	printf("Div by zero - Are you nuts!\n");
}

/*
 * handle_page_fault_exception
 *   DESCRIPTION: Jump table entry from exception_handler and prints page fault exception
 *   INPUTS: none
 *   OUTPUTS: Prints page fault error depending on the value of the value in CR2
 * 				if NULL then prints NUll pointer exception and for other addresses then page fault exception
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clears screen  
 */ 
void handle_page_fault_exception(){

	int address = read_cr2();
	if(address == NULL)
		printf("Null pointer exception!\n");
	else
		printf("Page Fault Exception : %x\n",address);
}


void handle_invalid_tss(){
	clear();

	printf("Invalid TSS\n");
}

void handle_invalid_opcode(){
	clear();

	printf("Invalid Opcode\n");
}

void handle_invalid_ss(){
	clear();

	printf("Invalid SS\n");
}

void handle_invalid_segment(){
	clear();

	printf("Invalid SEGMENT\n");
}


/*
 * exception_handler
 *   DESCRIPTION: Jump table for handling exceptions
 *   INPUTS: exception number based on PIC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none 
 */ 
void exception_handler(int exception_number){

	switch(exception_number){

		case DIV_ERROR_EXCEPTION : 		handle_div_exception();
									break;
		case PAGE_FAULT_EXCEPTION :		handle_page_fault_exception();
									break;
		case INVALID_TSS:			handle_invalid_tss();
									break;
		case INVALID_SS:			handle_invalid_ss();
									break;				
		case INVALID_OPCODE:			handle_invalid_opcode();
									break;										
		case SEG_ABSENT:			handle_invalid_segment();
									break;										
		
		default :
					break;


	}



}
/* helper function that disables the irq and the eoi at the specified irq number*/
void mask_and_ack(int irq_num){

	disable_irq(irq_num);
	send_eoi(irq_num);
	
}


/*
 * exception_handler
 *   DESCRIPTION: Jump table for handling exceptions
 *   INPUTS: register values specified in the regs at the top of irq.c
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success -1 on failure
 *   SIDE EFFECTS: none 
 */ 
int sys_call_dispatcher(struct regs * r){
	
	int syscall_number = r->eax;
	int ret;
	uint8_t status;
	char* filename;
	switch(syscall_number){
		
		case EXECUTE :		filename = (char *)r->ebx;
							ret = execute((uint8_t *)filename);
							break;
		case OPEN	 :		filename = (char *)r->ebx;
							ret = open((uint8_t *)filename);
							break;
		case CLOSE	 :		ret = close((uint32_t)r->ebx);
							break;
							
		case READ	 :		sti();
							ret = read(r->ebx, (uint8_t *)r->ecx, (int)r->edx);
							break;
							
							
		case WRITE 	  :		ret = write(r->ebx, (uint8_t *)r->ecx, (int)r->edx);
							break;
		case HALT 	  : 	status=r->ebx&BITMASK_HALT_8;
							ret=halt(status)&BITMASK_HALT_16;
							break;
		case GETARGS  :		ret=getargs((uint8_t *)r->ebx,(int32_t)r->ecx);
							break;

		case VIDMAP   :		ret = vidmap((uint8_t **)r->ebx);
							break;
		default		  :		ret = -1;
							break;
		
	}
	
	
	return ret;
	
}

/*
 * handler
 *   DESCRIPTION: When exception or interrupt occurs this function handles it
 *   INPUTS: regs struct which is what the stack will look when this is called
 *   OUTPUTS: Prints exception at pid that occurred
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none  
 */ 
int handler(struct regs *r)
{
    /* Is this a fault whose number is from 0 to 31? */
    int i = 0;
    for(i = 0; i < MAX_N_PROCESSES; i++){
    	if(used_pids[i] != UNUSED_PID)
    		break;
    }

    // Check if the interrupt is a system call    
    if(i < MAX_N_PROCESSES && r->int_no == 0x80 ){
	cli();

    	// update current task's primary tss
    	pcb * current_task = current;

	tss_t * curr_tss = &current_task->hardware_context;
	curr_tss->ds = r->ds;
	curr_tss->cs = r->cs;
	curr_tss->es = r->es;
	curr_tss->fs = r->fs;
	curr_tss->gs = r->gs;
	curr_tss->edi = r->edi;
	curr_tss->esi = r->esi;
	curr_tss->ebp = r->ebp;
	curr_tss->esp = r->user_esp;
	curr_tss->ebx = r->ebx;
	curr_tss->edx = r->edx;
	curr_tss->ecx = r->ecx;
	curr_tss->eax = r->eax;
	curr_tss->eip = r->eip;
	curr_tss->eflags = r->eflags;
	if(r->ss != USER_DS)
		curr_tss->ss = KERNEL_DS;
	else
		curr_tss->ss = USER_DS;

	next_task = current_task;

  		

    	
    }
    else{

	// Else - it is a hardware interrupt. Update the secondary tss
    	cli();
    	
    	pcb * current_task = current;

    	tss_t * curr_tss = &current_task->secondary_hardware_context;
		
	curr_tss->ds = r->ds;
	curr_tss->cs = r->cs;
	curr_tss->es = r->es;
	curr_tss->fs = r->fs;
	curr_tss->gs = r->gs;
	curr_tss->edi = r->edi;
	curr_tss->esi = r->esi;
	curr_tss->ebp = r->ebp;
	curr_tss->esp = r->user_esp;
	curr_tss->ebx = r->ebx;
	curr_tss->edx = r->edx;
	curr_tss->ecx = r->ecx;
	curr_tss->eax = r->eax;
	curr_tss->eip = r->eip;
	curr_tss->eflags = r->eflags;

	if(r->ss != USER_DS){
		curr_tss->ss = KERNEL_DS;
		curr_tss->esp0 = r->esp + 0x14; // The addition of 0x14 is to set esp0 to point to the top of the stack. These 20 bytes register values pushed by the processor
						// which would be over-written later
	}
	else{
		curr_tss->ss = USER_DS;
		curr_tss->esp0 = current->hardware_context.esp0;
	}


	next_task = current_task;

    }

  

    if (r->int_no < 32)
    {
        /* Display the description for the Exception that occurred.
        *  In this tutorial, we will simply halt the system using an
        *  infinite loop */

        printf("Exception. pid : %d\n",current->pid);
        printf("Eip : %x\n",r->eip);
        printf("Esp : %x, User esp : %x\n", r->esp,r->user_esp);
        printf("cs : %x, ds : %x, es : %x, ss : %x\n", r->cs,r->ds,r->es,r->ss);
        printf("Eflags : %x\n",r->eflags);
        printf("Registers : edi : %x, esi : %x, ebp : %x\n eax : %x, ebx : %x, edx : %x, ecx : %x\n",r->edi,r->esi,r->ebp, r->eax, r->ebx, r->edx, r->ecx );
        
        exception_handler(r->int_no);
        while(1);
    }
	else if(r->int_no >= 32 && r->int_no < 48){
		 cli();
		mask_and_ack(r->int_no);
		(*interrupt_handlers_array[r->int_no - 32])();
		cli();
		enable_irq(r->int_no);
		if(next_task != 0){

			next_task->type_of_return = 1; // return from interrupt
			
			if(next_task != current){

				tss.esp0 = next_task->secondary_hardware_context.esp0;
				tss.ss0 = next_task->secondary_hardware_context.ss0;
				load_paging(next_task->pid);			// Load paging for the next task

			}
			else{
				load_paging(current->pid);			// Load paging for the current task
			}
			
		}
		return 1;

	}
	else if(r->int_no == 0x80){
		int ret;

		// call the appropriate system call
		ret = sys_call_dispatcher(r);
		cli();
		if(next_task != 0 ){

			// handle return from system call			
			if(next_task != current){
				next_task->type_of_return = 0;
				tss.esp0 = next_task->hardware_context.esp0;
				load_paging(next_task->pid);

			}
			else{
				current->type_of_return = 0;
				next_task->secondary_hardware_context.esp0 = current->hardware_context.esp0;
				tss.esp0 = current->hardware_context.esp0;
				load_paging(current->pid);


			}


		}
		return ret;
		
	}
	//should never end up here
	return 0;
}
