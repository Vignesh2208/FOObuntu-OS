/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
static uint8_t master_mask; /* IRQs 0-7 */
static uint8_t slave_mask; /* IRQs 8-15 */

#define MASTER_BEGIN 0x20
#define MASTER_END 0x27
#define SLAVE_BEGIN 0x28
#define SLAVE_END 0x2F

/* Initialize the 8259 PIC */
 /*
 * i8259_init
 *   DESCRIPTION: initialize PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none  
 */ 
void
i8259_init(void)
{
	/*Masking interrupt on both PICs*/
	outb(0xFF, 0x21);
	outb(0xFF, 0xA1);

	master_mask = 0xFF;
	slave_mask 	= 0xFF;

	/*Initializing Master PIC*/
	outb(ICW1, 0x20);
	outb(ICW2_MASTER, 0x21);
	outb(ICW3_MASTER, 0x21);
	outb(ICW4, 0x21);
	/*Initializing Slave PIC*/
	outb(ICW1, 0xA0);
	outb(ICW2_SLAVE, 0xA1);
	outb(ICW3_SLAVE, 0xA1);
	outb(ICW4, 0xA1);
}

/* Enable (unmask) the specified IRQ */
 /*
 * enable_irq
 *   DESCRIPTION: enables irq on the PIC based on the irq number passed in
 *   INPUTS: irq number to enable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */ 

void
enable_irq(uint32_t irq_num)
{
	if(MASTER_BEGIN <= irq_num && irq_num <= MASTER_END){
		master_mask = master_mask & (~(1 << (irq_num - MASTER_BEGIN)));
		
		outb(master_mask, 0x21);
	}
	else if(SLAVE_BEGIN <= irq_num && irq_num <= SLAVE_END){
		slave_mask = slave_mask & (~(1 << (irq_num - SLAVE_BEGIN)));
		irq_num = 0x22;
		master_mask = master_mask & (~(1 << (irq_num - MASTER_BEGIN)));
		outb(slave_mask, 0xA1);
		outb(master_mask,0x21);
	}
	
}

/* Disable (mask) the specified IRQ */
 /*
 * disable_irq
 *   DESCRIPTION: disables irq on the PIC based on the irq number passed in
 *   INPUTS: irq number to disable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */ 
void
disable_irq(uint32_t irq_num)
{

	if(MASTER_BEGIN <= irq_num && irq_num <= MASTER_END){
		master_mask = master_mask | ((1 << (irq_num - MASTER_BEGIN)));
		outb(master_mask, 0x21);
	}
	else if(SLAVE_BEGIN <= irq_num && irq_num <= SLAVE_END){
		slave_mask = slave_mask | ((1 << (irq_num - SLAVE_BEGIN)));
		outb(slave_mask, 0xA1);
	}
}

/* Send end-of-interrupt signal for the specified IRQ */
 /*
 * send_eoi
 *   DESCRIPTION: sends EOI to ignore other interrupts
 *   INPUTS: irq number to enable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */ 
void
send_eoi(uint32_t irq_num)
{
	int irq = irq_num - MASTER_BEGIN;
	
	if (irq & 8) {
		inb(0xA1);					
		outb(slave_mask,0xA1);
		outb(EOI+(irq&7),0xA0);	
		outb(0x62,0x20);			 
	} else {
		inb(0x21);					
		outb(master_mask,0x21);
		outb(EOI+irq,0x20);		
	}
	// if(irq >= 8)
	// 	outb(0x20,SLAVE_8259_PORT);
 
	// outb(0x20,MASTER_8259_PORT);

}

