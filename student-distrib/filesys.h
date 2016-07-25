#ifndef _FILESYS_H
#define _FILESYS_H

#define DATABLOCK_ADDR_ARRAY_OFFSET 4
#define DENTRY_ARRAY_OFFSET 64
#define MAX_FILE_NAME_LEN 32
#define MAX_INODE_ENTRIES 64
#define SPECIAL_INODE_DIR 64
#define SPECIAL_INODE_RTC 65
#define INODE_SIZE 4096
#define DATA_BLOCK_SIZE 4096
#define READ_BUFF_LENGTH 701
#define MAGIC_NUMBER_ELF 0x464C457F
#define MAX_OPEN_FILES 8
#define FILE_TYPE_RTC 0
#define FILE_TYPE_DIR 1
#define FILE_TYPE_REG 2

#include "types.h"
#include "lib.h"
#include "pcb.h"

typedef struct inode{
	uint32_t length_in_bytes;
	uint32_t *data_block_index;
} inode_t;

typedef struct dentry{
	char file_name[32];
	uint32_t file_type;
	uint32_t inode_index;
	uint32_t reserved[6];
} __attribute__((packed)) dentry_t;

typedef struct boot_block_entries{
	uint32_t num_dentries;
	uint32_t num_inodes;
	uint32_t num_data_blocks;
} boot_block_entries_t;

typedef struct boot_block{
	boot_block_entries_t *boot_block_entry_struct;
	dentry_t *dentries_array;
} boot_block_t;

void filesys_init(void);
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t** dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t** dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t write_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
uint32_t ret_inode_length (uint32_t inode);

int32_t ret_num_dentries (void);
int32_t print_file (int32_t fd, uint32_t mode, uint32_t offset, uint32_t length);
void ls(void);
int32_t direc_read(uint32_t fd, uint8_t *buf, int size);
int32_t file_open(uint8_t* fname);
int32_t file_close(int32_t fd);
uint32_t file_write(uint32_t fd, uint8_t *buf, int size);
uint32_t file_read(uint32_t fd, uint8_t *buf, int size);

#endif
