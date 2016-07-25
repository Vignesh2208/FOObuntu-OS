.text
.globl load_page_directory
#loads the page directory pointer into cr3
load_page_directory:
pushl %ebp
movl %esp, %ebp
movl 8(%esp), %eax
movl %eax, %cr3
movl %ebp, %esp
popl %ebp
ret

.globl enable_paging
#turn on the enable paging bit of cr0
enable_paging:
pushl %ebp
movl %esp, %ebp
movl %cr0, %eax
orl $0x80000000, %eax
movl %eax, %cr0
movl %ebp, %esp
popl %ebp
ret

#turn off th enable paging bit of cr0
.globl disable_paging
disable_paging:
pushl %ebp
movl %esp, %ebp
movl %cr0, %eax
andl $0x7fffffff, %eax
movl %eax, %cr0
movl %ebp, %esp
popl %ebp
ret

#turn on the enable pse of cr4
.globl enable_pse
enable_pse:
pushl %ebp
movl %esp, %ebp
movl %cr4, %eax
orl $0x00000010, %eax
movl %eax, %cr4
movl %ebp, %esp
popl %ebp
ret

#turn off th enable pse of cr4
.globl disable_pse
disable_pse:
pushl %ebp
movl %esp, %ebp
movl %cr4, %eax
andl $0xffffffef, %eax
movl %eax, %cr4
movl %ebp, %esp
popl %ebp
ret
