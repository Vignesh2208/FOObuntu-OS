#include "rtc.h"
#include "../pcb.h"
volatile uint32_t rtc_interrupt_flag;
extern void test_interrupts();

 /*
 * rtc_init
 *   DESCRIPTION:initialize rtc 
 *   INPUTS: irq number to enable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: frequency set to 1028hz
 */ 
void rtc_init(){

	outb(REGB_NMI_DISABLE,RTC_CMOS);		// select register B, and disable NMI
	char prev=inb(RTC_NMI);	// read the current value of register B
	outb(REGB_NMI_DISABLE,RTC_CMOS);		// set the index again (a read will reset the index to register D)
	outb(prev | 0x40,RTC_NMI);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
	
	//adjust the frequency of the RTC
	outb(REGA_NMI_DISABLE,RTC_CMOS);				// set index to register A, disable NMI
	char prev1=inb(RTC_NMI);			// get initial value of register A
	outb( REGA_NMI_DISABLE,RTC_CMOS);				// reset index to A
	outb((prev1 & 0xF0) | 0xF,RTC_NMI);//write only our rate to A. Note, rate is the bottom 4 bits.
										// change value here of RTC (0-disable)  1->F fast->slow
}
 /*
 * rtc_irq_handler
 *   DESCRIPTION: what to do when interrupt occurs
 *   INPUTS: irq number to enable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls test_interrupts which flashes the screen
 */ 
void rtc_irq_handler(){
	outb(0x8C,RTC_CMOS);			// Read Register C to clear the IRQF flag which allows the next interrupt to occur	
	char curr;
	curr = inb(RTC_NMI);
	rtc_interrupt_flag=0;
	// test_interrupts();
	
	outb(RTC_CMOS, 0x0C);	// select register C
	inb(RTC_NMI);			// just throw away contents
} 
 /*
 * open
 *   DESCRIPTION: open rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *   SIDE EFFECTS: initialize rtc
 */ 
int rtc_open(const uint8_t* filename){
	//initialize rtc then always return 0
	rtc_init();
	return 0;
}
 /*
 * close
 *   DESCRIPTION: close rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *   SIDE EFFECTS: 
 */ 
int rtc_close(int32_t fd){
	//nothing to do always return 0
	return 0;
}
 /*
 * read
 *   DESCRIPTION: read rtc, or wait until another rtc interrupt occurs
 *   INPUTS: file descriptor, buff= ptr to NMI, nbytes number of bytes
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: waits
 */ 
int rtc_read(uint32_t fd, void * buff,uint32_t nbytes){
	rtc_interrupt_flag=1;
	//terminal_write(current->curr_terminal,"here\n",5);
	while (rtc_interrupt_flag==1);
	return 0;
}
/*
 * write
 *   DESCRIPTION: write to rtc, or change the frequency of the rtc
 *   INPUTS: file descriptor, buff= ptr to NMI, nbytes number of bytes
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int rtc_write(uint32_t fd, const void* buff,uint32_t index){

	uint32_t rate;
	switch(*(uint32_t*)buff){
		case HZ_2:
			rate=DIV_HZ_2;
			break;
		case HZ_4:
			rate=DIV_HZ_4;
			break;
		case HZ_8:
			rate=DIV_HZ_8;
			break;
		case HZ_16:
			rate=DIV_HZ_16;
			break;
		case HZ_32:
			rate=DIV_HZ_32;
			break;
		case HZ_64:
			rate=DIV_HZ_64;
			break;
		case HZ_128:
			rate=DIV_HZ_128;
			break;
		case HZ_256:
			rate=DIV_HZ_256;
			break;
		case HZ_512:
			rate=DIV_HZ_512;
			break;
		case HZ_1024:
			rate=DIV_HZ_1024;
			break;
		default:
			return -1;

	}

	//adjust the frequency of the RTC (rate 6 is fastest 15 is slowest )
	outb(REGA_NMI_DISABLE,RTC_CMOS);				// set index to register A, disable NMI
	char prev1=inb(RTC_NMI);			// get initial value of register A
	outb( REGA_NMI_DISABLE,RTC_CMOS);				// reset index to A
	outb((prev1 & 0xF0) | rate,RTC_NMI);//write only our rate to A. Note, rate is the bottom 4 bits.
	
	return 0;
}
