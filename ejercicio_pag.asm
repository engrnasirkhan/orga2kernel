BITS 16
%define ORIGIN 0x1200
global start
start:
		cli
 		jmp bienvenida
		
bienvenida:
	lgdt 	[GDT_DESC]			
	;seteamos el bit PE del registro cr0
	mov 	eax, cr0
	or  	eax, 01h
	mov 	cr0, eax		
	;segundo segmento en la GDT, el primero es nulo
 	jmp 	0x08:modo_protegido

BITS 32
modo_protegido:
	
 	mov 	ax, 0x10
 	mov 	ds, ax		;acomodo el segmento de datos antes de hacer lio
 	mov 	es, ax		;acomodo el segmento de datos antes de hacer lio
 	mov 	fs, ax		;acomodo el segmento de datos antes de hacer lio
	mov 	gs, ax		;acomodo el segmento de datos antes de hacer lio
 	mov	ss, ax		;acomodo el segmento de pila antes de usarla

	
	lidt [IDT_DESC]
	push dword 0x28
	push dword 0x20
	call init_pics
	add esp, 8

	sti
	sti
	
	lea eax, [protegido]
	push eax
	call print

	mov eax, 10
	int 0x0
lop:
	
	dec eax
	jmp lop
	
%include "segments.mac"
%include "gdt.asm"
%include "idt.asm"
%include "messages.asm"
%include "print_protegido.asm"
%include "remap_pic.asm"

	
;; 	call	idtFill
	
;; 	lidt	[idt_desc]
		

;; 		imprimir_protegido iniciando_juan, iniciando_juan_len
				
;; 		genero un GP
;; 		mov ax, 0
;; 		mov es, ax
;; 		mov [es:0x0], edi

;; 		mov eax, begin_page_directory
;; 		mov cr3, eax
				
;; 		mov eax, cr0
;;     		or eax, 0x80000000
;; 		mov cr0, eax
;; modo_prot_pag:		
;; 		imprimir_paginado estoy_paginado, estoy_paginado_len
	
;; 		mov [es:edi], edi

;; 		.cicloazul:
;; 			;xchg bx, bx
;;  			stosw
;; 			loop 	.cicloazul
;; 			mov 	ecx, mensaje_len
;; 		mov 	edi, ((10 * 80) + 13) << 1
		
;; 		mov 	ah, 0x1A
;; 		mov 	esi, mensaje
		
;; 		.ciclo:
;; 			lodsb
;; 			stosw
;; 			loop .ciclo
			
		
;; 		jmp 	$
		
;; mensaje:	db 'Mi vieja mula ya no es lo que era! (ahora protegidos :) )'
;; mensaje_len equ $ - mensaje

;; gp_handler:
;;     ; nos salteamos el error code
;;     imprimir_protegido gp_exception, gp_exception_len
;;     add esp, 4
;;     iret

;; pf_handler:
;;     imprimir_protegido pf_exception, pf_exception_len
;;     add esp, 4
;;     iret


;; %include "a20.asm"




