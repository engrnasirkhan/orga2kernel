#include <asm/types.h>
#include <asm/asm.h>
#include <asm/gdt.h>
#include <asm/idt.h>
#include <asm/handlers.h>
#include <kernel/cmdline.h>
#include <kernel/globals.h>
#include <boot/multiboot.h>
#include <screen/screen.h>
//#include <asm/page.h>

extern void kmain( multiboot_info_t * ) __noreturn;
void kinit ( multiboot_info_t* mbd, unsigned int magic ) __init;

void kinit ( multiboot_info_t* mbd, unsigned int magic ) {
	// Inicializamos el video.
	// Hacer esto primero parece ridículo, pero es necesario
	// para poder imprimir en pantalla, y dado que en este
	// punto tenemos un sistema 100% funcional y un entorno
	// "robusto" podemos hacerlo sin problemas.
	// Después de todo tenemos una GDT cargada, interrupciones
	// deshabilitadas, paginación activada con el kernel
	// mapeando toda la memoria física a partir de 0x80000000.
	set_screen_mode ( BIOS );
	set_screen_pointer ( (uint8_t *) 0x800b8000 );
	kclrscreen();

	if ( magic != 0x2BADB002 ) {
		// TODO: Hacer un kputs.
		kprint ( "Warning: No se inicializo con GRUB, intentando de todas formas.\n" );
	}

	// Inicializamos la pila.
	// NOTA: No creo que haga falta inicializar la pila
	// de momento apunta a 640KB sin utilizar la memoria virtual
	// pero pienso que da igual, en cuenta corramos procesos esta
	// pila se va a perder para siempre, el kernel no necesita
	// pila propia fuera del contexto de una tarea. ¿O si?

	// Inicializamos la GDT.
	gdtr_t *gdtr = (gdtr_t *) g_GDT;
	gdtr->limit = (uint16_t) (sizeof( g_GDT ) - 1);
	gdtr->base = (uint32_t)g_GDT - 0x80000000; // Dirección física.

	gdt_fill_code_segment( g_GDT + 1, (void *) 0, 0xFFFFFFFF, 0 ); // Código kernel
	gdt_fill_data_segment( g_GDT + 2, (void *) 0, 0xFFFFFFFF, 0 ); // Datos kernel
	gdt_fill_code_segment( g_GDT + 3, (void *) 0, 0xFFFFFFFF, 3 ); // Código usuario
	gdt_fill_data_segment( g_GDT + 4, (void *) 0, 0xFFFFFFFF, 3 ); // Datos usuario
	// Por lo que entiendo, LGDT puede leer una dirección virtual.
	lgdt( (void *) ((unsigned long) g_GDT - 0x80000000) ); // Dirección física

	// Inicializamos la IDT.
	g_IDTr.limit = sizeof(g_IDT) - 1;
	g_IDTr.base = (uint32_t) g_IDT - 0x80000000;
	// Por lo que entiendo, LIDT puede leer una dirección virtual.
	lidt( (void *) ((unsigned long) &g_IDTr - 0x80000000) ); // Dirección física
	init_isrs();
	init_irqs();
	sti();

	// Inicializamos el cmdline
	cmdline_init ( (const char *) mbd->cmdline );

	// Inicializamos paginación

	// Inicializamos scheduler

	// Saltamos a kmain
	kmain( mbd );
}
