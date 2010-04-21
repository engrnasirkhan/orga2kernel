.sect .text

# TODO: No guardar los registros que la convención de C no exige.
# ssize_t write( int fd, const void *buf, size_t size )
#.global write
#write:
#	pushl %ebx
#	movl $1, %eax # write
#	movl 8(%esp), %ecx
#	movl 12(%esp), %edx
#	movl 16(%esp), %ebx
#	int $0x80
#	popl %ebx
#	ret
