#ifndef _ISR_H
#define _ISR_H

#define DIV_ERROR_EXCEPTION 0x0
#define PAGE_FAULT_EXCEPTION 0x0E
#define RTC_INT	0x28
#define PIT_INT 0x20
#define KEYBOARD_INT 0x21
#define INVALID_OPCODE 0x6
#define INVALID_TSS 0xA
#define INVALID_SS 0xC
#define SEG_ABSENT 0xB

#define MAX_INT_NUM 16
//array of funcitons of interrupt handlers
void (* interrupt_handlers_array[MAX_INT_NUM])(void);

#endif
