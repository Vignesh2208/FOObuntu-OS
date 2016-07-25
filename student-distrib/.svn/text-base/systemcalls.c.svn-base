#include "paging.h"
#include "x86_desc.h"
#include "filesys.h"
#include "systemcalls.h"

#define FOUR_KB 0x1000
extern pcb* head_runqueue;
extern pcb* active_terminal_task ;
tss_t global_tss;
/*
 * get_current
 *   DESCRIPTION: Macro that gets the current process pcb
 *   INPUTS: none
 *   OUTPUTS: None
 *   RETURN VALUE: current pcb
 *   SIDE EFFECTS: none
 */ 
pcb * get_current(){
	struct pcb *current_val;
	
	__asm__("andl %%esp,%0; ":"=r" (current_val) : "0" (~8191UL));
	return (pcb *)current_val;

}
/*
 * find_task_by_pid
 *   DESCRIPTION: Helper function for the terminal switching that gets the 
 * 				pcb of the specified pid input
 *   INPUTS: pid (in terminal switch this is the base shell pid number)
 *   OUTPUTS: none
 *   RETURN VALUE: pcb pointer to the specified pid
 *   SIDE EFFECTS: none
 */
pcb * find_task_by_pid(pid_t search_pid){

	if(used_pids[search_pid] == UNUSED_PID)
		return NULL;

	 pcb * task;
     uint32_t * kernel_stack_addr = (uint32_t *)(PAGE_TABLE_ARRAY_BASE_ADDR - 4 - search_pid*KERNEL_STACK_SIZE);
     task = (pcb *)((uint8_t*)(kernel_stack_addr) - KERNEL_STACK_SIZE + 4);
     return task;
}
/*
 * update_return_address
 *   DESCRIPTION: updates the return address by the specified pcb of the task to return
 *   INPUTS: pcb of the task to return to
 *   OUTPUTS: None
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */ 
void update_return_address(pcb * task_to_return){
	
	uint32_t * curr_kernel_mode_stack_top = (uint32_t *)(((uint8_t *)current) + KERNEL_STACK_SIZE - 4);
		
	*curr_kernel_mode_stack_top = task_to_return->hardware_context.ss;	
	*(curr_kernel_mode_stack_top - 1) = task_to_return->hardware_context.esp;
	*(curr_kernel_mode_stack_top - 2) = task_to_return->hardware_context.eflags;
	*(curr_kernel_mode_stack_top - 3) = task_to_return->hardware_context.cs;
	*(curr_kernel_mode_stack_top - 4) = task_to_return->hardware_context.eip;
	
	
}
/*
 * halt
 *   DESCRIPTION: halts the current task 
 *   INPUTS: status (not used)
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: reruns new instance of shell if base shell is exited
 *				Also updates the runqueue accordingly
 */ 
int halt(uint8_t status){
	
	//decrement pid from array
	
	pid_t current_pid;
	current_pid=current->pid;
	int ind;
	//close all files open in the current process
	if(current->num_files_open>0){
		for(ind=0;ind<MAX_N_FILES;ind++){
			close(ind);
		}
	}	
	used_pids[current_pid]=UNUSED_PID;
	//fetch parent and child processes and connect them
	pcb * parent=current->parent;
	if (parent==NULL){
		//used_pids[current_pid]=current_pid;
		//terminal_close((terminal_t *)current->curr_terminal);
		if (current==head_runqueue)
			head_runqueue=current->runqueue;
		else{
			pcb* pcb_ptr=head_runqueue;
			while(pcb_ptr != NULL && pcb_ptr->runqueue != current){
		 		pcb_ptr = pcb_ptr->runqueue;
		 	}
		 	pcb_ptr->runqueue = current->runqueue;
		 	current->runqueue=NULL;

		}
		clear_screen(current->curr_terminal);
		execute((uint8_t *)"shell");
		show_screen(current->curr_terminal);
		return 0;
	}
	
	//check if current process is shell
	if (current->is_shell){
		//clear_screen((terminal_t *)current->curr_terminal);
		//terminals[current_pid]=(terminal_t);
		//terminal_close((terminal_t *)current->curr_terminal);
		if(current == active_terminal_task)
		show_screen((terminal_t *)parent->curr_terminal);
	}

	//next_task=parent;
	parent->child=current->child;
	//attatch the child's parent pcb as the parent of current pcb
	// if(parent->child!=NULL){
	// 	parent->child->parent=parent;
	// }

	//set the stack stuff to the parent's TSS
	/*
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &parent->hardware_context, tss_size);
		tss_desc_ptr = the_tss_desc;		
		ltr(KERNEL_TSS);//maybe enough to jump back
	}*/

	//reassign stack stuff
	tss.esp0 = parent->hardware_context.esp0;
	tss.ss0 = parent->hardware_context.ss0;
	
	if(current == active_terminal_task)
		active_terminal_task = parent;
	
	//runqueue reassignment
	 pcb* pcb_ptr;
	 pcb_ptr = head_runqueue;
	 if(current == head_runqueue){
	 	head_runqueue = current->runqueue;
	 	current->runqueue=NULL;
	 	parent->runqueue = head_runqueue;
	 	head_runqueue = parent;
	 }
	 else{
	 	while(pcb_ptr != NULL && pcb_ptr->runqueue != current){
	 		pcb_ptr = pcb_ptr->runqueue;
	 	}
	 	pcb_ptr->runqueue = current->runqueue;
	 	parent->runqueue = head_runqueue;
	 	head_runqueue = parent;
	 	current->runqueue=NULL;
	}


	//cli();
	//next_task=current->runqueue;
	// if(parent == active_terminal_task){
	next_task=parent;
	load_paging(parent->pid);
	

	// }
	// else{
	// 	next_task = active_terminal_task;
	// }

	return 0;

}
/*
 * execute
 *   DESCRIPTION: executes the program specified by the command 
 *   INPUTS: string command (this will be separated into actual command and argument)
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success -1 on error
 *   SIDE EFFECTS: Updates runqueue and handles buffer overflow if either command or argument 
 * 				is greater than MAX_ARG_SIZE or MAX_CMD_SIZE
 */ 
int execute(const uint8_t* command){
	uint8_t arg[MAX_ARG_SIZE];
	uint8_t cmd[MAX_CMD_SIZE];
	int i = 0;
	pid_t new_pid;
	//initialize arg to all end character
	int ind;

	for (ind=0;ind<MAX_CMD_SIZE;ind++){
		arg[ind]='\0';//fill both temporary arg and cmd with null character
		cmd[ind]='\0';
	}
	//check for cases of input command
	//if command points to null
	if (command==NULL){
		return -1;
	}
	
	//store the argument into arg array of characters
	ind=0;
	int command_length=0;
	
	while(command[ind] != '\0'	&& command[ind] != ' '&& command[ind] != '\n'&&command_length<MAX_CMD_SIZE){		//dont want space in the lookup
		cmd[ind]=command[ind];
		command_length++;
		ind++;
	}
	ind=0; 
	int arg_length=0;
	
	if (command[command_length]!='\0' && command[command_length] != '\n'){
		while(command[ind+command_length+1] != '\0'	&& command[ind+command_length+1] != ' '
			&& command[ind+command_length+1] != '\n'&&arg_length<MAX_ARG_SIZE){		//dont want space in the lookup
			arg[ind]=command[ind+command_length+1];
			arg_length++;
			ind++;
		}
	}
	

	dentry_t *temp;
	int retval;
	retval=read_dentry_by_name(cmd,&temp);
	if(retval==-1){
		return -1;
	}

	
	if (temp->file_type != 2){
		printf("file_type not valid file_type was %d\n",temp->file_type);
		return -1;
	}


	uint8_t elf_from_file[4];
	uint32_t inode_num = temp->inode_index;
	retval=read_data(inode_num,0,elf_from_file,4);
	if(retval==-1){
	
		printf("could not read data from %s\n",cmd);
		return -1;
	}
	//check if the ELF in the file has magic number x7f and "ELF"
	if(elf_from_file[0]!=0x7f||elf_from_file[1]!=0x45||elf_from_file[2]!=0x4c||elf_from_file[3]!=0x46){
		printf("File not have valid ELF\n");
		return -1;
	}
	uint8_t entry_addr[4];
	uint32_t exec_entry_addr = 0;
	retval=read_data(inode_num,24,entry_addr,4);
	if (retval==-1){
		printf("Could not retreive entry point\n");
		return -1;
	}
	for(i = 0; i < 4; i++){
		exec_entry_addr += (entry_addr[i] << 8*(i));
	}

	int len = ret_inode_length(temp->inode_index);

	
	
	// assign a pid here
	new_pid = UNUSED_PID;
	for(i = 0; i < MAX_N_PROCESSES; i++){
		if(used_pids[i] == UNUSED_PID){
			new_pid = i;
			used_pids[i] = i;
			break;
		}
		
	}

	if(new_pid == UNUSED_PID)
		return -1;

	
	
	//load paging scheme here
	load_paging(new_pid);	
	
	
	uint8_t * exec_load_address = (uint8_t *)(IMAGE_ADDR|IMAGE_OFF);
	uint8_t * page_start_address = (uint8_t *)IMAGE_ADDR;
	used_pids[new_pid] = new_pid;
	read_data(temp->inode_index,0,exec_load_address,len);
	
	
	
	//set up the kernel stack and hardware_context (tss) here for this process
	uint32_t * kernel_stack_addr = (uint32_t *)(PAGE_TABLE_ARRAY_BASE_ADDR - 4 - new_pid*KERNEL_STACK_SIZE);
	pcb * curr_pcb = (pcb *)((uint8_t*)(kernel_stack_addr) - KERNEL_STACK_SIZE + 4);
	curr_pcb->pid = new_pid;
	curr_pcb->hardware_context.esp0 = (uint32_t)kernel_stack_addr;
	curr_pcb->hardware_context.ss0 = KERNEL_DS;
	curr_pcb->hardware_context.eip = exec_entry_addr;
	curr_pcb->hardware_context.cs = USER_CS; 
	// not sure
	curr_pcb->hardware_context.eflags = (1 << INTERRUPT_ENABLE_FLAG_BIT) | (1 << 1);
	curr_pcb->hardware_context.esp = (uint32_t)page_start_address + FOUR_MB - 4;
	curr_pcb->hardware_context.ss = USER_DS;
	curr_pcb->hardware_context.ds = USER_DS;

	// not sure
	curr_pcb->hardware_context.ldt_segment_selector = KERNEL_LDT;



	curr_pcb->secondary_hardware_context.esp0 = (uint32_t)kernel_stack_addr;
	curr_pcb->secondary_hardware_context.ss0 = KERNEL_DS;
	curr_pcb->secondary_hardware_context.eip = exec_entry_addr;
	curr_pcb->secondary_hardware_context.cs = USER_CS; 
	// not sure
	curr_pcb->secondary_hardware_context.eflags = (1 << INTERRUPT_ENABLE_FLAG_BIT) | (1 << 1);
	curr_pcb->secondary_hardware_context.esp = (uint32_t)page_start_address + FOUR_MB - 4;
	curr_pcb->secondary_hardware_context.ss = USER_DS;
	curr_pcb->secondary_hardware_context.ds = USER_DS;

	// not sure
	curr_pcb->secondary_hardware_context.ldt_segment_selector = KERNEL_LDT;


	//copy the args
	ind=0;
	for (ind=0;ind<MAX_ARG_SIZE;ind++){
		if(ind<arg_length)
			curr_pcb->args[ind]=arg[ind];
		else{
			curr_pcb->args[ind]='\0';
			break;
		}
	}
	
	//check to see if task is a shell
	curr_pcb->child=NULL;
	if(strncmp((int8_t *)cmd,(int8_t *)"shell",command_length)==0){
		curr_pcb->is_shell=1;
		if(curr_pcb->pid < MAX_CONCURRENT_TERMINALS){
			curr_pcb->parent=NULL;
		}
		else{
			curr_pcb->parent=current;
			current->child = curr_pcb;			
		}

		curr_pcb->curr_terminal = &terminals[new_pid];
		terminal_open((terminal_t *)curr_pcb->curr_terminal);
		
	}
	else{
		curr_pcb->is_shell=0;
		curr_pcb->parent=current;
		current->child = curr_pcb;
		curr_pcb->curr_terminal = &terminals[curr_pcb->parent->pid];
	}

	/*if(new_pid == MAX_CONCURRENT_TERMINALS - 1){
		tss.esp0 = curr_pcb->hardware_context.esp0;
		tss.ss0 = KERNEL_DS;
	}*/

	/*
	// load new tss into GDT
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &curr_pcb->secondary_hardware_context, tss_size);
		tss_desc_ptr = the_tss_desc;		
		ltr(KERNEL_TSS);
	}*/

	for(i=0; i< MAX_N_FILES; i++){
		(curr_pcb->fd_array[i]).is_used = 0;
	}

	if(current == active_terminal_task)
		active_terminal_task = curr_pcb;
	
	if(head_runqueue == NULL){
		curr_pcb->runqueue = NULL;
		head_runqueue = curr_pcb;
		active_terminal_task = head_runqueue;
	}
	else{
		//curr_pcb->runqueue = current->runqueue;
		//current->runqueue = curr_pcb;
		curr_pcb->runqueue  = head_runqueue;
		head_runqueue = curr_pcb;
		pcb* temp = head_runqueue;

		if(current->child != NULL){
			while(temp != NULL){
				if(temp->runqueue == current){
					temp->runqueue = current->runqueue;
					current->runqueue = NULL;
					break;
				}
				temp = temp->runqueue;
			}
		}

	}

	// cli();

	next_task = curr_pcb;
	return 0;
}

/*
 * open
 *   DESCRIPTION: called in irq.c (sys_call_dispatcher) to open a specified file
 *				with the file name specified by filename
 *   INPUTS: string filename
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success -1 on error
 *   SIDE EFFECTS: handles open with file type not by file name
 */ 
int32_t open(const uint8_t * filename){

	uint8_t vacant_fd;
	for(vacant_fd = 0; ((vacant_fd < MAX_N_FILES) &&
				 ((current->fd_array[vacant_fd]).is_used != 0)); vacant_fd++);
	if(vacant_fd == MAX_N_FILES){
		return -1;

	}

	// if(strncmp((int8_t *) filename, (int8_t *)"rtc", 3) == 0){
	// 	rtc_open(filename); //Input does nothing

	// }

	(current->fd_array[vacant_fd]).inode = file_open((uint8_t *) filename);

	if((current->fd_array[vacant_fd]).inode == SPECIAL_INODE_RTC){
		(current->fd_array[vacant_fd]).fops_close = (func_close) rtc_close;
		(current->fd_array[vacant_fd]).fops_read = (func_read) rtc_read;
		(current->fd_array[vacant_fd]).fops_write = (func_write) rtc_write;
		(current->fd_array[vacant_fd]).offset = 0;
		(current->fd_array[vacant_fd]).inode = -1;
		(current->fd_array[vacant_fd]).is_used = 1;
		return (int32_t) vacant_fd+2;
	}

	if((current->fd_array[vacant_fd]).inode == -1){
		//terminal_write(current->curr_terminal,"File open returns -1\n",strlen("File open returns -1\n"));
		return -1;
	}

	(current->fd_array[vacant_fd]).fops_close = (func_close) file_close;
	(current->fd_array[vacant_fd]).fops_read = (func_read) file_read;
	(current->fd_array[vacant_fd]).fops_write = (func_write) file_write;
	(current->fd_array[vacant_fd]).offset = 0;
	(current->fd_array[vacant_fd]).is_used = 1;
	return (int32_t) vacant_fd+2;
}
/*
 * write
 *   DESCRIPTION: called in irq.c (sys_call_dispatcher) to write to buf length size to file
 *				using file descriptor 
 *   INPUTS: file descriptor fd, buffer buf, size to write 
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success -1 on error
 *   SIDE EFFECTS: none
 */ 
int32_t write(uint32_t fd, uint8_t * buf, int size){
	if(fd == 0)
		return -1;
	if(fd == 1 && next_task != 0){ // write to process's current terminal
		//printf("Called terminal write %d\n",current->pid);
		terminal_write((terminal_t *)current->curr_terminal, (char *)buf, size);
		return size;
	}
	else if(fd == 1)
		return -1;

	fd -= 2;
	if (fd >= MAX_N_FILES || current->fd_array[fd].is_used == 0)
		return -1;
	//terminal_write(current->curr_terminal,"Write\n",6);
	return (current->fd_array[fd].fops_write)(fd, buf, size);
}
/*
 * read
 *   DESCRIPTION: called in irq.c (sys_call_dispatcher) to read from file with fd
 * 				and read values to buffer until size
 *   INPUTS: file descriptor fd, buffer buf, size to write 
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success -1 on error
 *   SIDE EFFECTS: none
 */ 
int32_t read(uint32_t fd, uint8_t * buf, int size){
	if(fd == 1)
		return -1;
	if(fd == 0 && next_task != 0){ // write to process's current terminal
		//printf("Called terminal read\n");
		return terminal_read((terminal_t *)current->curr_terminal, (char *)buf, size);
	}
	else if(fd == 0)
		return -1;

	fd -= 2;
	if (fd >= MAX_N_FILES || current->fd_array[fd].is_used == 0)
		return -1;

	//terminal_write(current->curr_terminal,"Read\n",5);
	//int ret;
	return (current->fd_array[fd].fops_read)(fd, buf, size);
	//terminal_write(current->curr_terminal,"Return\n",7);
	//return ret;
}
/*
 * close
 *   DESCRIPTION: called in irq.c (sys_call_dispatcher) close file
 *				using file descriptor 
 *   INPUTS: file descriptor fd
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success -1 on error
 *   SIDE EFFECTS: none
 */ 
int32_t close(uint32_t fd){
	if(fd <= 1)
		return -1;

	fd -= 2;
	if (fd >= MAX_N_FILES || current->fd_array[fd].is_used == 0)
		return -1;

	current->fd_array[fd].is_used = 0;
	return (current->fd_array[fd].fops_close)(fd);

}
/*
 * close
 *   DESCRIPTION: called in irq.c (sys_call_dispatcher) write argument
 *			to buf with specified nbytes 
 *   INPUTS: file descriptor fd
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success -1 on error
 *   SIDE EFFECTS: handles if nbytes is greater than MAX_ARG_SIZE
 */ 
int32_t getargs(uint8_t * buf, int32_t nbytes){
	
	int copy_len=MAX_ARG_SIZE;
	if(nbytes <= copy_len)
		copy_len = nbytes;

	if(current->args[0]==(uint8_t)'\0')
		return -1;
	memcpy(buf,current->args,copy_len-1);
	buf[copy_len] = '\0';
	return 0;
}
/*
 * close
 *   DESCRIPTION: called in irq.c (sys_call_dispatcher) maps video memory
 *   INPUTS: pointer to array of the video data
 *   OUTPUTS: None
 *   RETURN VALUE: 0 on success -1 on error
 *   SIDE EFFECTS: none
 */ 
int32_t vidmap(uint8_t ** screen_start){
	if((uint32_t)(screen_start) >=IMAGE_ADDR && (uint32_t)(screen_start) <= (IMAGE_ADDR + FOUR_MB - 4)){
		// if (current==active_terminal_task){
			clear_screen(current->curr_terminal);
			*screen_start = (uint8_t *)VIDEO;
		// }
		return 0;
	}

	return -1;


}
