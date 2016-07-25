/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "drivers/drivers.h"
#include "isr.h"
#include "paging.h"
#include "filesys.h"
#include "systemcalls.h"
#include "pit.h"

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

extern idt_desc_t idt[NUM_VEC];
extern void isr_rtc();
extern void isr_keyboard();
extern void isr_div_error();
extern void isr_page_fault();
extern void isr_system_call();
extern void isr_invalid_opcode();
extern void isr_invalid_TSS();
extern void isr_invalid_SS();
extern void isr_invalid_SEG_ABSENT();
extern int call_execute(char* name);
extern void isr_pit();

struct idt_load{

	uint16_t size;
	uint32_t address;
};

//struct to hold all 48 bits of data of the IDT
void setup_idt(uint32_t handler_address, int irq_num)
{



	idt[irq_num].offset_15_00 = ((int)handler_address & 0xffff);
	idt[irq_num].offset_31_16 = (((int)handler_address >> 16) & 0xffff);
	idt[irq_num].seg_selector = KERNEL_CS;
	idt[irq_num].reserved4 = 0;
	idt[irq_num].reserved3 = 0;
	idt[irq_num].reserved2 = 1;
	idt[irq_num].reserved1 = 1;
	idt[irq_num].size = 1;
	idt[irq_num].reserved0 = 0;
	if(irq_num == 0x80)
		idt[irq_num].dpl = 3;
	else
		idt[irq_num].dpl = 0;
	
	idt[irq_num].present = 1;


}

char *filesys_addr; //Start address of the filesystem image
char filesys_string[] = "/filesys_img";

terminal_t kernel_terminal;

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
	multiboot_info_t *mbi;

	/* Clear the screen. */
	clear();

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	printf ("flags = 0x%#x\n", (unsigned) mbi->flags);
	
	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
		printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2))
		printf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (CHECK_FLAG (mbi->flags, 3)) {
		int mod_count = 0;
		int i;
		module_t* mod = (module_t*)mbi->mods_addr;
		while(mod_count < mbi->mods_count) {
			printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
			printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
			printf("First few bytes of module:\n");
			if(strncmp((int8_t *)filesys_string, (int8_t *)(mod->string), strlen((int8_t *)filesys_string)) == 0)
				filesys_addr = (char *)mod->mod_start; //Storing the start address of file system in memory
			for(i = 0; i<16; i++) {
				printf("0x%x ", *((char*)(mod->mod_start+i)));
			}
			printf("\n");
			mod_count++;
			mod++;
		}
	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		printf ("elf_sec: num = %u, size = 0x%#x,"
				" addr = 0x%#x, shndx = 0x%#x\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
					"     type = 0x%x,  length    = 0x%#x%#x\n",
					(unsigned) mmap->size,
					(unsigned) mmap->base_addr_high,
					(unsigned) mmap->base_addr_low,
					(unsigned) mmap->type,
					(unsigned) mmap->length_high,
					(unsigned) mmap->length_low);;
	}

	
	/* Construct an LDT entry in the GDT */
	{
		seg_desc_t the_ldt_desc;
		the_ldt_desc.granularity    = 0;
		the_ldt_desc.opsize         = 1;
		the_ldt_desc.reserved       = 0;
		the_ldt_desc.avail          = 0;
		the_ldt_desc.present        = 1;
		the_ldt_desc.dpl            = 0x0;
		the_ldt_desc.sys            = 0;
		the_ldt_desc.type           = 0x2;

		SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
		ldt_desc_ptr = the_ldt_desc;
		lldt(KERNEL_LDT);
	}

	/* Construct a TSS entry in the GDT */
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

		SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

		tss_desc_ptr = the_tss_desc;

		tss.ldt_segment_selector = KERNEL_LDT;
		tss.ss0 = KERNEL_DS;
		tss.esp0 = 0x800000;
		ltr(KERNEL_TSS);
	}

	clear();	

	/*Load IDT here?*/	
	setup_idt((uint32_t)isr_div_error,DIV_ERROR_EXCEPTION);
	setup_idt((uint32_t)isr_page_fault,PAGE_FAULT_EXCEPTION);
	setup_idt((uint32_t)isr_rtc,RTC_INT);
	setup_idt((uint32_t)isr_pit,PIT_INT);
	setup_idt((uint32_t)isr_keyboard,KEYBOARD_INT);
	setup_idt((uint32_t)isr_system_call,0x80);
	setup_idt((uint32_t)isr_invalid_SEG_ABSENT,SEG_ABSENT);
	setup_idt((uint32_t)isr_invalid_opcode,INVALID_OPCODE);
	setup_idt((uint32_t)isr_invalid_TSS,INVALID_TSS);
	setup_idt((uint32_t)isr_invalid_SS,INVALID_SS);
	

	
	lidt(idt_desc_ptr);
	/*loading the interrupt handler array*/
	
	interrupt_handlers_array[RTC_INT - 32] = &rtc_irq_handler;
	interrupt_handlers_array[KEYBOARD_INT - 32] = &keyboard_irq_handler;
	interrupt_handlers_array[PIT_INT - 32] = &pit_irq_handler;
	
	/* Init the PIC */
	i8259_init();

	/*Initializing devices*/
	enable_irq(KEYBOARD_INT);
	
	/* RTC TEST */
	//enable rtc init and enable irq to test interrupt
	//rtc_init();
	
	// unmask interrupt for rtc
	enable_irq(RTC_INT);
	
	pit_init();

	enable_irq(PIT_INT);

	//Enable filesystem
	filesys_init();
	

	/* KEYBOARD TEST */
	//char *out_String;
	// terminal_t kernel_terminal;
	/*terminal_open(&kernel_terminal);
	clear_screen(&kernel_terminal);
	out_String = "IDT loaded\n";
	terminal_write(&kernel_terminal, "IDT loaded\n",strlen(out_String));	
	out_String = "IRQ handlers loaded\n";
	terminal_write(&kernel_terminal, out_String,strlen(out_String));
	out_String = "IRQ handlers loaded\n";
	terminal_write(&kernel_terminal, out_String,strlen(out_String));	
	out_String = "Devices initialized\n";	
	terminal_write(&kernel_terminal, out_String,strlen(out_String));
	*/

	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */

	/* Enable interrupts */
	/* Do not enable the following until after you have set up your
	 * IDT correctly otherwise QEMU will triple fault and simple close
	 * without showing you any output */
	/*out_String = "Enabling Interrupts\n";
	terminal_write(&kernel_terminal, out_String,strlen(out_String));*/
	sti();

	/*out_String = "Press CTRL-L to clear screen >>";
	terminal_write(&kernel_terminal, out_String,strlen(out_String));*/


	// Initialize pid arrays
	int i;
	next_task = 0;
	for(i = 0; i < MAX_N_PROCESSES; i++){
		used_pids[i] = UNUSED_PID;
		
	}






	/*Loading in kernel page directory and corresponding page tables*/
	/*Enalbing paging in CPU*/
	
	//load_paging();
	
	/*###############################################################*/
	/*Terminal/Keybiard testing*/
	// char read_data[100];
	// while(1){	
	// 	terminal_read(&kernel_terminal, read_data,100);
	// 	out_String = "Data Read : \n";
	// 	terminal_write(&kernel_terminal, out_String,strlen(out_String));
	// 	terminal_write(&kernel_terminal,read_data,strlen(read_data));
	// }
	// terminal_close(&kernel_terminal);
	
	/*###############################################################*/
	/*RTC Testing*/
	// terminal_close(&kernel_terminal);
	// uint8_t *test;
	// rtc_open(test);
	
	// uint32_t dummy_ptr=0;
	// uint32_t a[1];
	// uint32_t b=0;
	// uint32_t freq=2;
	// for (freq=2;freq<=1024;freq*=2){
	// 	a[0]=freq;
 //    	void *test_buff = (void *)a;
		
	// 	int ind=0;
	// 	for (ind=0;ind<10;ind++){
	// 		rtc_read(dummy_ptr,test_buff,b);
	// 		printf("%d",ind);
	// 	}
	// 	printf("\n");
	// 	rtc_write(dummy_ptr,test_buff,b);
		
	// 	// out_String = "RTC written to new value\n";
	// 	// terminal_write(&kernel_terminal, out_String,strlen(out_String));
	// }
	// a[0]=2;
 //    void *test_buff = (void *)a;
	// rtc_write(dummy_ptr,test_buff,b);
	// rtc_close(dummy_ptr);
	
	/*###############################################################*/
	/*File system testing*/

	//Read and print files
	// int32_t fd = file_open((uint8_t*)"frame0.txt");
	// if(fd != -1){
	// 	print_file(fd, 0, 0, 0);
	// 	file_close(fd);
	// }
	// else{
	// 	printf("Error opening file\n");
	// }
	
	// Printing without opening
	//print_file(0,1, 43 + 2268, 500);

	// int32_t fd = file_open((uint8_t*)"verylargetxtwithverylongname.tx");
	// if(fd != -1){
	// 	print_file(fd, 0, 0, 0);
	// 	// print_file(fd,1, 43 + 2268, 2048);
	// 	file_close(fd);
	// }
	// else{
	// 	printf("Error opening file\n");
	// }	
	
	// int32_t fd = file_open((uint8_t*)"cat");
	// if(fd != -1){
	// 	print_file(fd, 1, 0, 1024);
	// 	file_close(fd);
	// }
	// else{
	// 	printf("Error opening file\n");
	// }	

	// int32_t fd = file_open((uint8_t*)"rtc");
	// if(fd != -1){
	// 	print_file(fd, 0, 0, 0);
	// 	file_close(fd);
	// }
	// else{
	// 	printf("Error opening file\n");
	// }	

	//Read and print directoruy
	terminal_close(&kernel_terminal);
	clear_screen(&kernel_terminal);
	//ls();

	//int *ptr = NULL;
	//*ptr = 1;	

	/* Execute the first program (`shell') ... */
	
	//execute((uint8_t*)"shell");
	char * shell_file_name = "shell";
	int ret;
	int j = 0;
	for(j = 0 ; j < (MAX_CONCURRENT_TERMINALS-1); j++){
		execute((uint8_t *)shell_file_name);
	}
	ret = call_execute(shell_file_name);
	printf("Return value = %d\n",ret);
	
	

	/* Spin (nicely, so we don't chew up cycles) */
	asm volatile(".1: hlt; jmp .1;");
}

