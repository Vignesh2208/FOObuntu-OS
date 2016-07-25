#ifndef _PAGING_H
#define _PAGING_H

#include "lib.h"

int load_paging();
void load_task(uint32_t address);
void flush_tlb();
void vid_page_map(uint32_t pid);
#define TABLES_PER_DIRECTORY 1024
#define PAGES_PER_TABLE 1024

#define PAGE_TABLE_ARRAY_BASE_ADDR 0x800000
#define KERNEL_4M_PAGE_ADDR 0x400000

//struct to hold all 32 bits of data of the page directory entry
struct page_directory_entry{
	uint8_t present : 1;
	uint8_t read_write : 1;
	uint8_t user_supervisor : 1;
	uint8_t write_through : 1;
	uint8_t cache_disabled : 1;
	uint8_t accessed : 1;
	uint8_t ignored2 : 1;
	uint8_t page_size : 1;
	uint8_t ignored1 : 1;
	uint8_t avail : 3;
	uint32_t address : 20;
}  __attribute__((packed));
//struct to hold all 32 bits of data of the page table entry
struct page_table_entry{
	uint8_t present : 1;
	uint8_t read_write : 1;
	uint8_t user_supervisor : 1;
	uint8_t write_through : 1;
	uint8_t cache_disabled : 1;
	uint8_t accessed : 1;
	uint8_t dirty : 1;
	uint8_t ignored1 : 1;
	uint8_t global : 1;
	uint8_t avail : 3;
	uint32_t address : 20;
}  __attribute__((packed));

#endif
