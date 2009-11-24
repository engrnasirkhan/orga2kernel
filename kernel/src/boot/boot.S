.equ MODULEALIGN, (1<<0)
.equ MEMINFO, (1<<1)
.equ FLAGS, (MODULEALIGN | MEMINFO)
.equ MAGIC, 0x1BADB002
.equ CHECKSUM, -(MAGIC + FLAGS)

.equ virtual_start, 0x80000000

.extern __kernel_end
.extern kinit

.section .init.text,"ax",@progbits
.section .init.rodata,"a",@progbits
.section .init.data,"aw",@progbits
.section .init.bss,"aw",@nobits

.sect .init.text
.align 4
	.long MAGIC
	.long FLAGS
	.long CHECKSUM

.align 16
.global boot
boot:
	movl $640*1024, %esp # 640KB para abajo

	lgdt (temporal_gdt-virtual_start)
	ljmp $8,$(continuar-virtual_start)
continuar:
	movl $16, %ecx
	movl %ecx, %ds
	movl %ecx, %es
	movl %ecx, %ss

	# Guardamos los parámetros del grub.
	# Ajustamos EBX para que el código C pueda
	# usar direcciones virtuales.
	addl $virtual_start, %ebx
	pushl %eax
	pushl %ebx

	# Activamos paginación utilizando páginas de 4MB
	# Mapeamos el kernel a 2GB = 0x80000000
	# De modo que el mapa queda así:
	# 0GB..2GB --> 0..2GB
	# 2GB..4GB --> 0..2GB
	movl $(activando_paginacion-virtual_start), %esi
	movl (linea-virtual_start), %edi
	call early_print
	movl (linea-virtual_start), %eax
	addl $80*2, %eax
	movl %eax, (linea-virtual_start)

	# Redondeamos al próximo múltiplo de 4KB
	movl $(__kernel_end-virtual_start), %edi
	addl $4095, %edi
	andl $~0xFFF, %edi

	# Rellenamos la primer parte de la tabla.
	movl $512, %ecx
	movl $0x83, %eax # Present, System, RW
1:
	movl %eax, (%edi)
	addl $4, %edi
	addl $4096*1024, %eax
	loop 1b

	# Rellenamos la segunda parte de la tabla.
	movl $512, %ecx
	movl $0x83, %eax
2:
	movl %eax, (%edi)
	addl $4, %edi
	addl $4096*1024, %eax
	loop 2b
	
	# Activamos páginas de 4MB
	movl %cr4, %eax
	orl $(1<<4), %eax
	movl %eax, %cr4

	# Establecemos el directorio de páginas
	movl $(__kernel_end-virtual_start), %eax
	addl $4095, %eax
	andl $~0xFFF, %eax
	movl %eax, %cr3

	# Activamos paginación
	movl %cr0, %eax
	orl $(1<<31), %eax
	movl %eax, %cr0

	ljmp $8, $3f
3:
	movl $paginacion_activada, %esi
	movl linea, %edi
	call early_print

	call kinit

	cli
4:
	hlt
	jmp 4b

# Copia la cadena de ESI a EDI.
early_print:
	movl $0x7f00, %eax
	cld
1:
	lodsb
	or %al, %al # ¿Ya terminamos?
	jz 2f # Si, entoces salir.
	stosw
	jmp 1b # Repetimos
2:
	ret

.sect .init.rodata
.align 16
temporal_gdt:
	.short fin_temporal_gdt - temporal_gdt - 1
	.long temporal_gdt - virtual_start
	.short 0

	.quad 0x00CF9A000000FFFF # Código X/R
	.quad 0x00CF92000000FFFF # Datos R/W
fin_temporal_gdt:

activando_paginacion: .string "Activando paginaci\xF3n..."
paginacion_activada: .string "Paginaci\xF3n activada."

.sect .init.data
linea: .long 0xb8000
