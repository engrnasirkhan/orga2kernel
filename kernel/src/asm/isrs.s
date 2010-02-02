.file "isr.s"
.sect .text

.macro ISRDECL nro, err=0
.global isr_\nro
isr_\nro:
	.ifeq \err
		pushl $0
	.endif
	pushl $\nro
	jmp isr_common
.endm

ISRDECL 0
ISRDECL 1
ISRDECL 2
ISRDECL 3
ISRDECL 4
ISRDECL 5
ISRDECL 6
ISRDECL 7
ISRDECL 8,1
ISRDECL 9
ISRDECL 10,1
ISRDECL 11,1
ISRDECL 12,1
ISRDECL 13,1
ISRDECL 14,1
ISRDECL 15
ISRDECL 16
ISRDECL 17,1
ISRDECL 18
ISRDECL 19
ISRDECL 20
ISRDECL 21
ISRDECL 22
ISRDECL 23
ISRDECL 24
ISRDECL 25
ISRDECL 26
ISRDECL 27
ISRDECL 28
ISRDECL 29
ISRDECL 30
ISRDECL 31

# 1) Guardamos todos los registros.
# 2) No necesitamos guardar DS, ES, FS, etc porque son siempre el DS de usuario que creamos antes
# 3) Además, no vamos a modificar DS porque sabemos que DS=0-4GB,DPL=3, lo cual se puede usar
#    perfectamente desde ring0.
# Si vemos que falla (2) o (3), modificar.
.extern isr_dispatch_routine
isr_common:
	pusha
	call isr_dispatch_routine
	popa
	addl $8, %esp
	iret
