#ifndef RTC_H
#define RTC_H
#include "../lib.h"

#define RTC_CMOS 0x70
#define RTC_NMI 0x71
#define REGB_NMI_DISABLE 0x8B
#define REGA_NMI_DISABLE 0x8A


#define HZ_2 0x2
#define HZ_4 0x4
#define HZ_8 0x8
#define HZ_16 16
#define HZ_32 32
#define HZ_64 64
#define HZ_128 128
#define HZ_256 256
#define HZ_512 512
#define HZ_1024 1024

#define DIV_HZ_2 15
#define DIV_HZ_4 14
#define DIV_HZ_8 13
#define DIV_HZ_16 12
#define DIV_HZ_32 11
#define DIV_HZ_64 10
#define DIV_HZ_128 9
#define DIV_HZ_256 8
#define DIV_HZ_512 7
#define DIV_HZ_1024 6
//initialize the rtc
void rtc_init();
//function when interrupt occurs
void rtc_irq_handler();
//opens rtc 
int rtc_open(const uint8_t* filename);
//close rtc, just return 0
int rtc_close(int32_t fd);
//read rtc, wait until next interrupt occurs
int rtc_read(uint32_t fd, void * buff,uint32_t index);
//change the frequency of the interrupt
int rtc_write(uint32_t fd, const void * buff,uint32_t index);
#endif
