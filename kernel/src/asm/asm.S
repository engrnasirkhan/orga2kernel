.sect .text

.global lgdt
lgdt:
	movl 4(%esp), %eax
	lgdt (%eax)
	ljmp $8, $1f # Asume CS = 8
1:
	movw $16, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	ret
