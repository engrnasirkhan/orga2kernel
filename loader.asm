global loader

extern kmain
extern GDT_DESC;
extern IDT_DESC;

MODULEALIGN equ 1<<0
MEMINFO	equ 1<<1
FLAGS	equ MODULEALIGN | MEMINFO
MAGIC	equ 0x1BADB002
CHECKSUM	equ -(MAGIC + FLAGS)

section	.text
	align 4
	
MultiBootHeader:
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

STACKSIZE equ 0x4000

loader:
	mov esp, stack+STACKSIZE

	;lidt  [IDT_DESC]	
	;lgdt  [GDT_DESC]
	
	push eax
	push ebx

	call kmain

	cli

hang:
	hlt
	jmp	hang

section .bss

	align 32
stack:
	resb STACKSIZE
