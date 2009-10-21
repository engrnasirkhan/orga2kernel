extern print
	
isr0:
	pusha
	
	lea eax, [isr0_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret
isr0_msg: db	"ISR 0",0	
isr1:
	pusha
	
	lea eax, [isr1_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret
isr1_msg: db	"ISR 1",0
isr2:
	pusha
	
	lea eax, [isr2_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret
isr2_msg: db	"ISR 2",0
isr3:
	pusha
	
	lea eax, [isr3_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret
isr3_msg: db	"ISR 3",0
isr4:
	pusha
	
	lea eax, [isr4_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret
isr4_msg: db	"ISR 4",0
isr5:
	pusha
	
	lea eax, [isr5_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret
isr5_msg: db	"ISR 5",0
isr6:
	pusha
	
	lea eax, [isr6_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret
isr6_msg: db	"ISR 6",0

isr7:
	pusha
	
	lea eax, [isr7_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret
isr7_msg: db	"ISR 7",0
isr8:
	pusha
	
	lea eax, [isr8_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret
isr8_msg: db	"ISR 8",0
isr9:
	pusha
	
	lea eax, [isr9_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret

isr9_msg: db	"ISR 9",0
isr10:
	pusha
	
	lea eax, [isr10_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret

isr10_msg: db	"ISR 10",0
isr11:
	pusha
	
	lea eax, [isr11_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret
isr11_msg: db	"ISR 11",0
isr12:
	pusha
	
	lea eax, [isr12_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret

isr12_msg: db	"ISR 12",0
isr13:
	pop eax 		; error code
	pusha
	lea eax, [isr13_msg]
	push eax
 	call print
	add esp, 4
	popa
	iret
isr13_msg: db	"ISR 13",0
isr14:
	pusha
	
	lea eax, [isr14_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret

isr14_msg: db	"ISR 14",0
isr15:
	pusha
	
	lea eax, [isr15_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret

isr15_msg: db	"ISR 15",0
isr32:
	pusha
	
	lea eax, [isr32_msg]
	push eax
	call print
	add esp, 4
	
	popa
	iret

isr32_msg: db	"ISR 32",0




