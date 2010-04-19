.file "syscall.s"
.sect .text

.extern syscall_table_entry
.global syscall_entry
syscall_entry:
	pushl $0
	pushl $0x80
	pusha
	call syscall_table_entry
	popa
	addl $8, %esp
	iret
