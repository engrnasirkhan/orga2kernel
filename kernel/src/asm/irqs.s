.file "irq.s"
.sect .text

.macro IRQDECL nro
.global irq_\nro
irq_\nro:
pushl $0
pushl $\nro
jmp irq_common
.endm

IRQDECL 0
IRQDECL 1
IRQDECL 2
IRQDECL 3
IRQDECL 4
IRQDECL 5
IRQDECL 6
IRQDECL 7
IRQDECL 8
IRQDECL 9
IRQDECL 10
IRQDECL 11
IRQDECL 12
IRQDECL 13
IRQDECL 14
IRQDECL 15

# 1) Guardamos todos los registros.
# 2) No necesitamos guardar DS, ES, FS, etc porque son siempre el DS de usuario que creamos antes
# 3) Además, no vamos a modificar DS porque sabemos que DS=0-4GB,DPL=3, lo cual se puede usar
#    perfectamente desde ring0.
# Si vemos que falla (2) o (3), modificar.
.extern irq_dispatch_routine
irq_common:
	pusha
	call irq_dispatch_routine
	popa
	addl $8, %esp
	iret
