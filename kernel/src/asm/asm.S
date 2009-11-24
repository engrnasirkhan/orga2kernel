.sect .text

.global lgdt
lgdt:
	movl 4(%esp), %eax
	lgdt (%eax)
	ljmp $8, $1f # Asume CS = 8
1:
	ret
