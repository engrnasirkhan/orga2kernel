
global boot           ; making entry point visible to linker
extern kmain         ; kinit is defined elsewhere
 
; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                   ; provide memory map
FLAGS       equ  MODULEALIGN | MEMINFO  ; this is the Multiboot 'flag' field
MAGIC       equ    0x1BADB002           ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum required

VIRTUAL_START_CORRECTION equ 0x80000000

section .data
    ;Esta GDT es temporal. Como linkeamos el kernel para que este a partir de los 2GB,
    ;las direccione virtuales estan referidas a ese comienzo, pero fisicamente el kernel se
    ;carga a partir de 1mb. Para que todo ande bien, en principio definimos una gdt-dummy
    ;cuyos segmentos tienen una base tal que cuando se produce la conversion de direccion
    ;virtual a direccion fisica, todo anda bien :)
    ;Mas tarde dentro del kernel se instala una gdt final, con el modelo flat y se activa 
    ;paginacion y se descarta esta gdt-dummy   

    dummy_gdt:
	    dd 0, 0
	    db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x80	; code selector 0x08: base 0x80000000, limit 0xFFFFFFFF, type 0x9A, granularity 0xCF
	    db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x80	; data selector 0x10: base 0x80000000, limit 0xFFFFFFFF, type 0x92, granularity 0xCF

    dummy_gdt_desc:
	    dw $ - dummy_gdt 
	    dd (dummy_gdt - VIRTUAL_START_CORRECTION)

section .text
    align 4
    MultiBootHeader:
       dd MAGIC
       dd FLAGS
       dd CHECKSUM
     
    ;Reservamos 16k para la pila del kernel
    STACKSIZE equ 0x4000                  ; that's 16k.
     
    boot:
       ;Cargamos la dummy-gdt para iniciar ejecutando correctamente
       lgdt [dummy_gdt_desc - VIRTUAL_START_CORRECTION ]
	   jmp 0x08:(segmented_boot)
    
    segmented_boot:
       mov cx, 0x10
       mov ds, cx
       mov es, cx
       mov ss, cx
       mov fs, cx
       mov gs, cx
       
       mov esp, stack+STACKSIZE           ; set up the stack
       push eax                           ; pass Multiboot magic number
       push ebx                           ; pass Multiboot info structure

       ;saltamos a kinit. Dentro de kinit iniciamos las cosas necesarias para poder 
       ;comenzar con la ejecucion del kernel usando paginacion
       call  kmain
     
       cli
    hang:
       hlt                                ; halt machine should kernel return
       jmp   hang

section .bss
    align 4
    stack:
       resb STACKSIZE                     ; reserve 16k stack on a doubleword boundary
