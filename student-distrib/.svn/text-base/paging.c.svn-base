#include "paging.h"
#include "x86_desc.h"
#include "systemcalls.h"

//static struct page_directory_entry page_directory[TABLES_PER_DIRECTORY] __attribute__((aligned (4*TABLES_PER_DIRECTORY)));
static struct page_directory_entry page_directory[MAX_N_PROCESSES][TABLES_PER_DIRECTORY] __attribute__((aligned (4*TABLES_PER_DIRECTORY)));
static int* first_free_page = (int *)PAGE_TABLE_ARRAY_BASE_ADDR;

extern pcb* active_terminal_task;

extern void load_page_directory(struct page_directory_entry*);
extern void enable_paging();
extern void disable_paging();
extern void enable_pse();
extern void disable_pse();
void flush_tlb(){
	asm volatile(
		"movl %%cr3, %%eax  \n \
		movl %%eax, %%cr3" \
		:\
		:\
		: "memory","cc" );
}


/*
 * load_paging
 *   DESCRIPTION: Load Page directory entry and page table entry
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *   SIDE EFFECTS: enable paging 
 */  
int load_paging(pid_t pid){
	int i;
	struct page_table_entry *curr_table_entry;
	disable_paging();
	for(i=0; i<TABLES_PER_DIRECTORY; i++){
		page_directory[pid][i].present = 0;			// preset=0
		page_directory[pid][i].read_write = 1; 		// enabled write
		page_directory[pid][i].user_supervisor = 0;
		page_directory[pid][i].write_through = 0;
		page_directory[pid][i].cache_disabled = 0;
		page_directory[pid][i].accessed = 0;
		page_directory[pid][i].ignored2 = 0;
		page_directory[pid][i].page_size = 0;
		page_directory[pid][i].ignored1 = 0;
		page_directory[pid][i].avail = 0;
		page_directory[pid][i].address = 0x0;
	}
	
	/*Loading the Kernel page which is 2nd entry in page directory*/
	page_directory[pid][1].present = 1;			// preset=0
	page_directory[pid][1].read_write = 1; 		// enabled write
	page_directory[pid][1].user_supervisor = 1;
	page_directory[pid][1].write_through = 0;
	page_directory[pid][1].cache_disabled = 0;
	page_directory[pid][1].accessed = 0;
	page_directory[pid][1].ignored2 = 0;
	page_directory[pid][1].page_size = 1;
	page_directory[pid][1].ignored1 = 0;
	page_directory[pid][1].avail = 0;
	page_directory[pid][1].address = KERNEL_4M_PAGE_ADDR >> 12;


	/*Loading the 0-4MB (1st) page directory entry*/
	page_directory[pid][0].present = 1;			// preset=0
	page_directory[pid][0].read_write = 1; 		// enabled write
	page_directory[pid][0].user_supervisor = 1;
	page_directory[pid][0].write_through = 0;
	page_directory[pid][0].cache_disabled = 0;
	page_directory[pid][0].accessed = 0;
	page_directory[pid][0].ignored2 = 0;
	page_directory[pid][0].page_size = 0;
	page_directory[pid][0].ignored1 = 0;
	page_directory[pid][0].avail = 0;
	page_directory[pid][0].address = PAGE_TABLE_ARRAY_BASE_ADDR >> 12;


	/*Loading the 0-4MB (1st) page directory entry*/
	page_directory[pid][32].present = 1;			// preset=0
	page_directory[pid][32].read_write = 1; 		// enabled write
	page_directory[pid][32].user_supervisor = 1;
	page_directory[pid][32].write_through = 0;
	page_directory[pid][32].cache_disabled = 0;
	page_directory[pid][32].accessed = 0;
	page_directory[pid][32].ignored2 = 0;
	page_directory[pid][32].page_size = 1;
	page_directory[pid][32].ignored1 = 0;
	page_directory[pid][32].avail = 0;
	page_directory[pid][32].address = (pid*FOUR_MB + PROGRAM_LOAD_START_ADDRESS) >> 12;


	/*Loading the first page table*/
	curr_table_entry = (struct page_table_entry *)first_free_page;
	for(i=0; i<PAGES_PER_TABLE; i++){
		curr_table_entry[i].present = 0;			// preset=0
		curr_table_entry[i].read_write = 1; 		// enabled write
		curr_table_entry[i].user_supervisor = 0;
		curr_table_entry[i].write_through = 0;
		curr_table_entry[i].cache_disabled = 0;
		curr_table_entry[i].accessed = 0;
		curr_table_entry[i].dirty = 0;
		curr_table_entry[i].ignored1 = 0;
		curr_table_entry[i].global = 0;
		curr_table_entry[i].avail = 0;
		curr_table_entry[i].address = 0x0;
	}

	/*VRAM page-1*/
	curr_table_entry[VIDEO >> 12].present = 1;			// preset=1
	curr_table_entry[VIDEO >> 12].read_write = 1; 		// enabled write
	curr_table_entry[VIDEO >> 12].user_supervisor = 1;
	curr_table_entry[VIDEO >> 12].write_through = 0;
	curr_table_entry[VIDEO >> 12].cache_disabled = 0;
	curr_table_entry[VIDEO >> 12].accessed = 0;
	curr_table_entry[VIDEO >> 12].dirty = 0;
	curr_table_entry[VIDEO >> 12].ignored1 = 0;
	curr_table_entry[VIDEO >> 12].global = 0;
	curr_table_entry[VIDEO >> 12].avail = 0;
	if(pid==active_terminal_task->pid)
		curr_table_entry[VIDEO >> 12].address = (VIDEO >> 12);
	else
		curr_table_entry[VIDEO >> 12].address = (VIDEO >> 12)+1;

	/*VRAM page-2*/
	curr_table_entry[(VIDEO >> 12) + 1].present = 1;			// preset=1
	curr_table_entry[(VIDEO >> 12) + 1].read_write = 1; 		// enabled write
	curr_table_entry[(VIDEO >> 12) + 1].user_supervisor = 1;
	curr_table_entry[(VIDEO >> 12) + 1].write_through = 0;
	curr_table_entry[(VIDEO >> 12) + 1].cache_disabled = 0;
	curr_table_entry[(VIDEO >> 12) + 1].accessed = 0;
	curr_table_entry[(VIDEO >> 12) + 1].dirty = 0;
	curr_table_entry[(VIDEO >> 12) + 1].ignored1 = 0;
	curr_table_entry[(VIDEO >> 12) + 1].global = 0;
	curr_table_entry[(VIDEO >> 12) + 1].avail = 0;

	if(pid==active_terminal_task->pid)
		curr_table_entry[(VIDEO >> 12) + 1].address = (VIDEO >> 12) + 1;
	else
		curr_table_entry[(VIDEO >> 12) + 1].address = (VIDEO >> 12);
	
	/*loads the page directory, enable pse and paging by setting registers*/
	load_page_directory(page_directory[pid]);
	enable_pse();
	enable_paging();

	return 0;

}

