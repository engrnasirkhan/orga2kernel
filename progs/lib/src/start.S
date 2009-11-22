# Este es el primer archivo de _TODOS_ los programas
# Primero se encuentra esta tabla:
.sect .text
.global __start

.long 0x00455845 # "EXE\0"
.long __start # Entry point :-)
.long __text_start
.long __data_start
.long __bss_start
.long __bss_end

__start:
	pushl $0 # argv
	pushl $0 # argc
	call main
	# Una excepción si llegamos acá.
1:
	cli
	hlt
	jmp 1b
