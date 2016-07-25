#include "filesys.h"
#include "drivers/drivers.h"

extern char *filesys_addr;
extern terminal_t kernel_terminal;

boot_block_t fs_boot_block;	
uint8_t is_inode_valid[MAX_INODE_ENTRIES]; //can be bool
//dentry_t* open_dentry_storage[MAX_OPEN_FILES];

const uint8_t hex_to_char[] = {'0', '1', '2', '3', '4', '5', '6', '7',
							   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void filesys_init(void){
	uint32_t i; // counter

	fs_boot_block.boot_block_entry_struct = (boot_block_entries_t *)filesys_addr;
	fs_boot_block.dentries_array = (dentry_t *)(filesys_addr + DENTRY_ARRAY_OFFSET);

	// Initialize the array to all zeros
	memset(is_inode_valid, 0, MAX_INODE_ENTRIES);
	// Setting valid inode validity to 1
	for(i = 0; i<(fs_boot_block.boot_block_entry_struct)->num_dentries; i++)
		if(((fs_boot_block.dentries_array)[i].file_type == FILE_TYPE_REG) && //checking if regular file
		  ((fs_boot_block.dentries_array[i]).inode_index < (fs_boot_block.boot_block_entry_struct)->num_inodes)) // if inode# id valid
				is_inode_valid[(fs_boot_block.dentries_array)[i].inode_index] = 1;

	return;
}

int32_t ret_num_dentries (void){
	return (fs_boot_block.boot_block_entry_struct)->num_dentries;
}

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t** dentry){
//What if someone tries to change the address
	uint32_t i;

	for(i = 0; i<(fs_boot_block.boot_block_entry_struct)->num_dentries; i++)
		if(strncmp((int8_t *)(fs_boot_block.dentries_array)[i].file_name, 
			(int8_t *)fname, MAX_FILE_NAME_LEN) == 0){
				*dentry = &((fs_boot_block.dentries_array)[i]);
				return 0;
		}

	//printf("fname:%s \n file not found error\n", fname);

	*dentry = NULL;
	return -1;
}

int32_t read_dentry_by_index (uint32_t index, dentry_t** dentry){
	if((fs_boot_block.boot_block_entry_struct)->num_dentries <= index){
		*dentry = NULL;
		return -1;
	}
	*dentry = &((fs_boot_block.dentries_array)[index]);
	return 0;
}

uint32_t ret_inode_length (uint32_t inode){
	//Checking if inode is valid
	if((inode >= (fs_boot_block.boot_block_entry_struct)->num_inodes) || (is_inode_valid[inode] == 0))
		return -1;
	
	return *((uint32_t *)(filesys_addr + (1+inode)*INODE_SIZE));
}

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	inode_t inode_entry;
	uint32_t i;
	uint32_t num_left = length;
	uint32_t num_copied = 0;
	char * curr_data_block_address;
	//Checking if inode is valid

	if((inode >= (fs_boot_block.boot_block_entry_struct)->num_inodes) || (is_inode_valid[inode] == 0)){
		//printf("if1\n");
		return -1;
	}

	inode_entry.length_in_bytes = *((uint32_t *)(filesys_addr + (1+inode)*INODE_SIZE));
	inode_entry.data_block_index = (uint32_t *)(filesys_addr + (1+inode)*INODE_SIZE + DATABLOCK_ADDR_ARRAY_OFFSET);
	//printf("inode length=%d, offset=%d, length=%d\n", inode_entry.length_in_bytes, offset, length);
	//Checking if offset and offset+length are valid
	if((offset >= inode_entry.length_in_bytes) ||
	  (offset+length - 1  >= inode_entry.length_in_bytes)){
		//printf("if2\n");
		return -1;
	}

	int first_block_index = offset/DATA_BLOCK_SIZE;
	int last_block_index = (offset + length - 1)/DATA_BLOCK_SIZE;
	int offset_from_block = offset - ((uint32_t)(offset/DATA_BLOCK_SIZE))*DATA_BLOCK_SIZE;

	//Checking if required data block indices are valid
	for(i = first_block_index; i <= last_block_index; i++)
		if((fs_boot_block.boot_block_entry_struct)->num_data_blocks <= (inode_entry.data_block_index)[i]){
			//printf("if3\n");
			return -1;	
		}

	// Copying if there is only one block
	if(first_block_index == last_block_index){
		curr_data_block_address = filesys_addr + ((fs_boot_block.boot_block_entry_struct)->num_inodes + 1)*INODE_SIZE + (inode_entry.data_block_index)[first_block_index]*DATA_BLOCK_SIZE;
		memcpy(buf, curr_data_block_address + offset_from_block, length);
		return length;
	}

	// Copying all but the last block
	for(i = first_block_index; i < last_block_index; i++){
		curr_data_block_address = filesys_addr + ((fs_boot_block.boot_block_entry_struct)->num_inodes + 1)*INODE_SIZE + (inode_entry.data_block_index)[i]*DATA_BLOCK_SIZE;
		if(i == first_block_index){ //Copying first block
			memcpy(buf + num_copied, curr_data_block_address + offset_from_block, DATA_BLOCK_SIZE - offset_from_block);
			num_copied +=  (DATA_BLOCK_SIZE - offset_from_block);
			num_left -=  (DATA_BLOCK_SIZE - offset_from_block);
		}
		else{
			memcpy(buf + num_copied, curr_data_block_address, DATA_BLOCK_SIZE);
			num_copied += DATA_BLOCK_SIZE;
			num_left -= DATA_BLOCK_SIZE;
		}

	}

	// Copying last block
	if(num_left > 0){ //Always TRUE anyways
		curr_data_block_address = filesys_addr + ((fs_boot_block.boot_block_entry_struct)->num_inodes + 1)*INODE_SIZE +  (inode_entry.data_block_index)[last_block_index]*DATA_BLOCK_SIZE;
		memcpy(buf + num_copied, curr_data_block_address, num_left)	;
		num_copied += num_left;
		num_left -= num_left;
	}

	return num_copied;

}

int32_t file_open(uint8_t* fname){
	if(strlen((int8_t*)fname) >= MAX_FILE_NAME_LEN)
		fname[MAX_FILE_NAME_LEN - 1] = '\0';

	if(strncmp((int8_t *)".", (int8_t *)fname, 1) == 0)
		return SPECIAL_INODE_DIR;

	dentry_t* dentry_ptr;

	//uint32_t open_dentry_counter = 0;
	// while(open_dentry_counter <= MAX_OPEN_FILES){
	// 	if(open_dentry_storage[open_dentry_counter] == 0){

		if(read_dentry_by_name (fname, &dentry_ptr) == -1)
			return -1;
		else if(dentry_ptr->file_type==FILE_TYPE_RTC){
			rtc_open(fname);
			return SPECIAL_INODE_RTC;
		}
		else if((dentry_ptr->inode_index >= (fs_boot_block.boot_block_entry_struct)->num_inodes)
			|| is_inode_valid[dentry_ptr->inode_index] == 0 
			|| ret_inode_length(dentry_ptr->inode_index) == 0){
			
			//printf("Error directory/file: %s not present OR\ninvalid inode entry OR\ndirectory file type OR\nEmpty File", fname);
			return -1;
		}
		
		return dentry_ptr->inode_index;
	// 	}
	// 	open_dentry_counter++;
	// }
	//printf("Maximum number of files opened already\n");
	//return -1;
}

int32_t file_close(int32_t fd){
	return 0;
}

// int32_t print_file (int32_t fd , uint32_t mode, uint32_t offset, uint32_t read_length){
// 	uint8_t read_buffer[READ_BUFF_LENGTH+1];
// 	read_buffer[READ_BUFF_LENGTH] = '\0';
// 	uint32_t file_length, length;
// 	uint32_t num_reads;
// 	uint32_t read_counter;
// 	int8_t *out_String;
// 	uint32_t i_counter;
// 	uint8_t elf_flag = 0;

// 	if(open_dentry_storage[fd] == 0){
// 		out_String = (int8_t *)"Wrong file descriptor passed\n";
// 		terminal_write(&kernel_terminal, (int8_t*)out_String, strlen((int8_t*)out_String));	
// 		//printf("Wrong file descriptor passed\n");
// 		return -1;
// 	}

// 	// if(strlen((int8_t*)fname) >= MAX_FILE_NAME_LEN)
// 	// 	fname[MAX_FILE_NAME_LEN - 1] = '\0';

// 	if((open_dentry_storage[fd])->file_type == FILE_TYPE_RTC){
// 		out_String = (int8_t *)"Error, cannot read RTC file\n";
// 		terminal_write(&kernel_terminal, out_String, strlen(out_String));
// 		// printf("Error, cannot read RTC file\n");
// 		return -1;
// 	}
// 	else if((open_dentry_storage[fd])->file_type == FILE_TYPE_DIR){
// 		out_String = (int8_t *)(open_dentry_storage[fd])->file_name;
// 		terminal_write(&kernel_terminal, out_String, strlen(out_String));
// 		out_String = (int8_t *)" is a Directory, cannot be printed. Use ls() instead\n";
// 		terminal_write(&kernel_terminal, out_String,strlen(out_String));		
// 		// printf("%s is a Directory, cannot be printed. Use ls() instead\n", (open_dentry_storage[fd])->file_name);
// 		return -1;
// 	}
// 	else if((open_dentry_storage[fd])->file_type == FILE_TYPE_REG){
// 		//printf("%s is Regular file\n", (open_dentry_storage[fd])->file_name);
// 		file_length = ret_inode_length((open_dentry_storage[fd])->inode_index);
// 		if(file_length == 0){
// 			out_String = (int8_t *)(open_dentry_storage[fd])->file_name;
// 			terminal_write(&kernel_terminal, out_String, strlen(out_String));
// 			out_String = (int8_t *)" is empty\n";
// 			terminal_write(&kernel_terminal, out_String, strlen(out_String));
// 			// printf("%s is empty\n", (open_dentry_storage[fd])->file_name);
// 			return 0;
// 		}
// 		else if(file_length == -1){
// 			out_String = (int8_t *)"Error in reading file length, inode index might be invalid\n";
// 			terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 			// printf("Error in reading file length, inode index:%d  might be invalid\n"
// 			// 	  , (open_dentry_storage[fd])->inode_index);
// 			return -1;
// 		}
// 		else if(file_length >= 4){
// 			if(read_data ((open_dentry_storage[fd])->inode_index, 0, read_buffer, 16) == -1){
// 				out_String = (int8_t *)"Invalid data block index found\n";
// 				terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 				// printf("\nInvalid data block index found\n");
// 				return -1;
// 			}
			
// 			if(*((uint32_t*)read_buffer) == MAGIC_NUMBER_ELF){
// 				// out_String = (uint8_t *)"Error: cannot print execuatble file using print_file\n";
// 				// terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 				// printf("Error: cannot print execuatble file using print_file\n");
// 				// return -1;

// 				out_String = (int8_t *)"ELF file found with magic number\n";
// 				terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 				//printf("ELF file found with magic number: %x\n",*((uint32_t*)read_buffer));

// 				elf_flag = 1;
// 			}
// 		}
		
// 		//printf("file length= %d\n", file_length);

// 		if(mode == 1){ //Mode 1 print a specific subset of data
// 			if( (offset < file_length) && (offset + read_length -1 < file_length))
// 				length = read_length;
// 			else{
// 				out_String = (int8_t *)"Offet or length out of range\n";
// 				terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 				// printf("Offet or length out of range\n");
// 				return -1;
// 			}
// 		}
// 		else if(mode == 0){  //Mode 0 prints everything
// 			length = file_length;
// 			offset = 0;
// 		}
// 		else{
// 			out_String = (int8_t *)"Print File: Wrong mode\n";
// 			terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 			// printf("Print File: Wrong mode\n");
// 			return -1;
// 		}

// 		if(elf_flag)
// 			offset += 4;

// 		num_reads = length/READ_BUFF_LENGTH;
// 		//printf("num reads= %d\n", num_reads);
// 		read_counter = 0;
// 		while(read_counter < num_reads){
// 			//printf("read_counter=%d\n", read_counter);
// 			if(read_data ((open_dentry_storage[fd])->inode_index, offset + (uint32_t)read_counter*READ_BUFF_LENGTH, read_buffer, READ_BUFF_LENGTH) == -1){
// 				out_String = (int8_t *)"Invalid data block index found\n";
// 				terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 				// printf("\nInvalid data block index found\n");
// 				return -1;
// 			}
// 			if(elf_flag){
// 				for(i_counter = 0; i_counter<READ_BUFF_LENGTH; i_counter++){
// 					write_character(&kernel_terminal, (char)hex_to_char[read_buffer[i_counter] & 0x0F]);
// 					write_character(&kernel_terminal, (char)hex_to_char[(read_buffer[i_counter] & 0xF0) >> 4]);
// 				}
// 			}
// 			else{
// 				out_String = (int8_t *)read_buffer;
// 				terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 				// printf("%s", read_buffer);
// 			}
// 			read_counter++;
// 		}		
// 		if(length - num_reads*READ_BUFF_LENGTH > 0){
// 			if(read_data ((open_dentry_storage[fd])->inode_index, offset +  (uint32_t)num_reads*READ_BUFF_LENGTH, read_buffer, length - num_reads*READ_BUFF_LENGTH) == -1){
// 				out_String = (int8_t *)"Invalid data block index found\n";
// 				terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 				// printf("\nInvalid data block index found\n");
// 				return -1;
// 			}
// 			read_buffer[length - num_reads*READ_BUFF_LENGTH] = '\0';
// 			if(elf_flag){
// 				for(i_counter = 0; i_counter<READ_BUFF_LENGTH; i_counter++){
// 					write_character(&kernel_terminal, (char)hex_to_char[read_buffer[i_counter] & 0x0F]);
// 					write_character(&kernel_terminal, (char)hex_to_char[(read_buffer[i_counter] & 0xF0) >> 4]);
// 				}
// 			}
// 			else{
// 				out_String = (int8_t *)read_buffer;
// 				terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 				// printf("%s", read_buffer);
// 			}
// 		}
// 	}
// 	else{
// 		out_String = (int8_t *)(open_dentry_storage[fd])->file_name;
// 		terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 		out_String = (int8_t *)" has unknown file type\n";
// 		terminal_write(&kernel_terminal, out_String,strlen(out_String));
// 		// printf("%s has unknown file type %d", (open_dentry_storage[fd])->file_name, (open_dentry_storage[fd])->file_type);
// 		return -1;
// 	}

// 	return 0;
// }

void ls(void){
	int32_t dentry_index = 0;
	int32_t read_return;
	dentry_t *dentry_storage;

	read_return = read_dentry_by_index(dentry_index, &dentry_storage);
	printf("\n\n\n");	
	printf(" (Dentry index) name {size} [file type]\n");	
	while(read_return != -1){
		printf("(%d)  %s  {%dB} [%d]\n",(int32_t)dentry_index, dentry_storage->file_name,
			   ret_inode_length(dentry_storage->inode_index), dentry_storage->file_type);
		dentry_index++;
		read_return = read_dentry_by_index(dentry_index, &dentry_storage);
	}
	printf("End of directory list\n");
	return;
}

int32_t direc_read(uint32_t fd, uint8_t *buf, int size){
	int32_t dentry_index = current->fd_array[fd].offset;
	int32_t read_return;
	dentry_t *dentry_storage;
	//printf("current offset %d\n", dentry_index);
	read_return = read_dentry_by_index(dentry_index, &dentry_storage);	
	if(read_return != -1){
		//printf("File read\n");
		strncpy((int8_t*)buf, (int8_t*)dentry_storage->file_name, (uint32_t)size);
		return strlen(dentry_storage->file_name);
	}
	return 0;
}

int32_t write_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	inode_t inode_entry;
	uint32_t i;
	uint32_t num_left = length;
	uint32_t num_copied = 0;
	char * curr_data_block_address;
	//Checking if inode is valid
	if((inode >= (fs_boot_block.boot_block_entry_struct)->num_inodes) || (is_inode_valid[inode] == 0))
		return -1;

	inode_entry.length_in_bytes = *((uint32_t *)(filesys_addr + (1+inode)*INODE_SIZE));
	inode_entry.data_block_index = (uint32_t *)(filesys_addr + (1+inode)*INODE_SIZE + DATABLOCK_ADDR_ARRAY_OFFSET);
	//printf("inode length=%d, offset=%d, length=%d\n", inode_entry.length_in_bytes, offset, length);
	//Checking if offset and offset+length are valid
	if((offset >= inode_entry.length_in_bytes) ||
	  (offset+length - 1  >= inode_entry.length_in_bytes))
		return -1;

	int first_block_index = offset/DATA_BLOCK_SIZE;
	int last_block_index = (offset + length - 1)/DATA_BLOCK_SIZE;
	int offset_from_block = offset - ((uint32_t)(offset/DATA_BLOCK_SIZE))*DATA_BLOCK_SIZE;

	//Checking if required data block indices are valid
	for(i = first_block_index; i <= last_block_index; i++)
		if((fs_boot_block.boot_block_entry_struct)->num_data_blocks <= (inode_entry.data_block_index)[i])
			return -1;	

	// Copying if there is only one block
	if(first_block_index == last_block_index){
		curr_data_block_address = filesys_addr + ((fs_boot_block.boot_block_entry_struct)->num_inodes + 1)*INODE_SIZE + (inode_entry.data_block_index)[first_block_index]*DATA_BLOCK_SIZE;
		memcpy(curr_data_block_address + offset_from_block, buf, length);
		return length;
	}

	// Copying all but the last block
	for(i = first_block_index; i < last_block_index; i++){
		curr_data_block_address = filesys_addr + ((fs_boot_block.boot_block_entry_struct)->num_inodes + 1)*INODE_SIZE + (inode_entry.data_block_index)[i]*DATA_BLOCK_SIZE;
		if(i == first_block_index){ //Copying first block
			memcpy(curr_data_block_address + offset_from_block, buf + num_copied, DATA_BLOCK_SIZE - offset_from_block);
			num_copied +=  (DATA_BLOCK_SIZE - offset_from_block);
			num_left -=  (DATA_BLOCK_SIZE - offset_from_block);
		}
		else{
			memcpy(curr_data_block_address, buf + num_copied, DATA_BLOCK_SIZE);
			num_copied += DATA_BLOCK_SIZE;
			num_left -= DATA_BLOCK_SIZE;
		}

	}

	// Copying last block
	if(num_left > 0){ //Always TRUE anyways
		curr_data_block_address = filesys_addr + ((fs_boot_block.boot_block_entry_struct)->num_inodes + 1)*INODE_SIZE +  (inode_entry.data_block_index)[last_block_index]*DATA_BLOCK_SIZE;
		memcpy(curr_data_block_address, buf + num_copied, num_left)	;
		num_copied += num_left;
		num_left -= num_left;
	}

	return num_copied;

}

uint32_t file_read(uint32_t fd, uint8_t *buf, int size){

	int32_t num_bytes_read = 0; 
	uint32_t file_size;
	if(current->fd_array[fd].inode >= SPECIAL_INODE_DIR){
		num_bytes_read = direc_read(fd, buf, size);
		if(num_bytes_read != 0)
			current->fd_array[fd].offset += 1;
		return num_bytes_read;
	}


	file_size = ret_inode_length (current->fd_array[fd].inode);

	if(size > file_size - current->fd_array[fd].offset)
		size = file_size - current->fd_array[fd].offset;

	num_bytes_read = read_data(current->fd_array[fd].inode, 
		current->fd_array[fd].offset, buf, size);
	//printf("num_bytes_read=%d\n", num_bytes_read);
	//printf("fd=%d, offset=%d\n", fd, current->fd_array[fd].offset);
	if (num_bytes_read <= 0)
		return 0;

	current->fd_array[fd].offset += num_bytes_read;
	return (uint32_t) num_bytes_read;
}

uint32_t file_write(uint32_t fd, uint8_t *buf, int size){
	return 0; 
}


