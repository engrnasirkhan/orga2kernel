#include <asm/types.h>
#include <asm/asm.h>
#include <asm/gdt.h>
#include <asm/idt.h>
#include <asm/handlers.h>
#include <kernel/cmdline.h>
#include <kernel/globals.h>
#include <screen/screen.h>
#include <drivers/pic8259A.h>
#include <mem/mmu.h>

//#include <asm/page.h>

//extern void kmain( multiboot_info_t * ) __noreturn;
void kinit ( multiboot_info_t* mbd ) __init;

void kinit ( multiboot_info_t* mbd ) {
	// Inicializamos el video.
	// Hacer esto primero parece ridículo, pero es necesario
	// para poder imprimir en pantalla, y dado que en este
	// punto tenemos un sistema 100% funcional y un entorno
	// "robusto" podemos hacerlo sin problemas.
	// Después de todo tenemos una GDT cargada, interrupciones
	// deshabilitadas, paginación activada con el kernel
	// mapeando toda la memoria física a partir de 0x80000000.
	set_screen_mode ( BIOS );
	set_screen_pointer ( (uint8_t *) PA2KVA(0x000b8000) );
	kclrscreen();

	// TODO: Copiamos los módulos bien lejos del kernel
	/*if ( mbd->flags & 8 ) {
		module_t *m;
		unsigned long i;

		for ( i = 0, mod = (module_t *) mbd->mods_addr;
			i < mbd->mods_count;
			i++, mod++ ) {
			unsigned long inicio = mod->mod_start, fin = mod->mod_end;
		}
	}*/

	// Inicializamos la pila.
	// NOTA: No creo que haga falta inicializar la pila
	// de momento apunta a 640KB sin utilizar la memoria virtual
	// pero pienso que da igual, en cuenta corramos procesos esta
	// pila se va a perder para siempre, el kernel no necesita
	// pila propia fuera del contexto de una tarea. ¿O si?

	// Inicializamos la mmu
    mmu_init(mbd);

	// Inicializamos la IDT.
	g_IDTr.limit = sizeof(g_IDT) - 1;
	//g_IDTr.base = (uint32_t) g_IDT - 0x80000000;
	g_IDTr.base = KVA2PA((uint32_t)g_IDT);
	// Por lo que entiendo, LIDT puede leer una dirección virtual.
	//lidt( (void *) ((unsigned long) &g_IDTr - 0x80000000) ); // Dirección física
	lidt( (void *) ( KVA2PA((unsigned long)&g_IDTr))); // Dirección física
	init_isrs();
	init_irqs();
	init_syscalls();
	pic8259A_init();

	// Inicializamos el cmdline
	cmdline_init ( (const char *) mbd->cmdline );


	// Inicializamos scheduler

}
