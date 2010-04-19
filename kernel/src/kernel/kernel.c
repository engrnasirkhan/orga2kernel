#include <asm/asm.h>
#include <asm/types.h>
#include <asm/handlers.h>
#include <boot/multiboot.h>
#include <kernel/panic.h>
#include <screen/screen.h>
#include <asm/gdt.h>
#include <asm/idt.h>
#include <asm/handlers.h>
#include <kernel/globals.h>
#include <mem/memlayout.h>
#include <mem/vmm.h>
#include <lib/string.h>
#include <drivers/keyboard.h>
#include <tty/tty.h>
#include <scheduler/scheduler.h>
#include <mem/mmu.h>
#include <drivers/pic8259A.h>

//funcion que inicializa gran parte de las estructuras del kernel
extern void kinit ( multiboot_info_t* mbd ) __init;

/* ebx = fd de salida.
 * ecx = puntero (en espacio de usuario).
 * edx = tamaño.
 */
int sys_write( struct registers *r ) {
	int i;
	const char *str = (const char *) r->ecx;
	if ( r->ebx != 1 ) return -1;

	/* if ( chequear_direccion(r->ecx, r->edx) == MALA ) return -1; */
	for ( i = 0; i < r->edx; i++ )
		kputc( str[i] );
	return r->edx;
}

/* Nota, hay que mandarle el EOI al PIC porque scheduler() NO vuelve,
 * Si no le mandamos el EOI nosotros no se lo manda nadie.
 * Además, hay que mandarle el EOI antes de hacer el cambio de contexto,
 * si lo hacemos después lo vamos a estar haciendo dos veces.
 * Una vez cuando nos vuelve a tocar la ejecución y volvemos de scheduler()
 * y la otra cuando volvemos al despachador de interrupciones.
 */
int timer( struct registers *r ) {
	#define TIEMPO_ACTUALIZCION 250  //Totalmente arbitrario
	if (tarea_activa == -1) {
		pic8259A_send_EOI( r->nro );
		scheduler();
	}
		
	//Referente a la actualizacion de pantalla activa
	/*++contador_actualizar_pantalla;
	if(contador_actualizar_pantalla == TIEMPO_ACTUALIZCION) { 
		contador_actualizar_pantalla == 0;
		if( tarea_en_pantalla == -1 ) menu();
		else mostrar_slot(tarea_en_pantalla);
	}*/
	
	
	//Referente a la decrementacion de quantum de tarea_activa
	//Decrementamos quantum
	--tareas[tarea_activa].quantum_actual;
	
	//si termino, reestablecemos y cambiamos a la proxima llamando a scheduler
	if (tareas[tarea_activa].quantum_actual<=0){		
			//Restablecemos quantums gastado
			tareas[tarea_activa].quantum_actual = tareas[tarea_activa].quantum_fijo; 
			//Llamamos al scheduler para que elija proxima tarea
			pic8259A_send_EOI( r->nro );
			scheduler();
	}

	return 0;
}

int teclado( struct registers *r ) { return 0; }

int pf( struct registers *r ) {
	cli();
	kprint( r->errcode & 1 ? "Page level protection\n" : "Page not present\n" );	
	kprint( r->errcode & 2 ? "Write error\n" : "Read error\n" );
	kprint( r->errcode & 4 ? "User mode\n" : "Supervisor mode\n" );	
	kprint( r->errcode & 8 ? "Reserved bits en 1 en PD\n" : "Reserved bits en 0 en PD\n" );	
	kprint( r->errcode & 16 ? "Instruction Fetch\n" : "No fue Instruction Fetch\n" );	
	kprint( "CR2: 0x%x\nDir: 0x%x:0x%x\n",
		getCR2(),
		r->cs, r->eip );
	for (;;) hlt();
}

void pruebatarea()  { for (;;) kprint("HOLA"); }
void pruebatarea2() { for (;;) kprint("chau"); }


void kmain(multiboot_info_t*, unsigned int magic ) __noreturn;
void kmain(multiboot_info_t* mbd, unsigned int magic ){
    if ( magic != 0x2BADB002 ) {
		// TODO: Hacer un kputs.
		kprint ( "Warning: No se inicializo con GRUB, intentando de todas formas.\n" );
	}

    //Primero tenemos que corregir mbd para que sea una direccion virtual valida
    mbd = (multiboot_info_t*)( PA2KVA( (uint32_t) mbd) );
    
    //Ahora si, usando mbd que apunta bien a los datos del grub, inicializamos todo
    kinit( mbd );

#if 0
    kprint("llego\n");
    key_init();
    key_register(menu, 1);
    key_register(mostrar_slot, 59);
    key_register(mostrar_slot, 60);
    key_register(mostrar_slot, 61);
    key_register(mostrar_slot, 62);
    key_register(mostrar_slot, 63);
    key_register(mostrar_slot, 64);
    key_register(mostrar_slot, 65);
    key_register(mostrar_slot, 66);
    key_register(mostrar_slot, 67);
    key_register(mostrar_slot, 68);
    kprint("binding keys done\n");
    tty_t kernel_tty;
    if(tty_init(&kernel_tty, NULL)){
        panic("fallo  el inicio de las tty");
    }
#endif
	//Lanzamos programa para cargar tareas y modificar quantums.
	set_isr_handler( 14, &pf ); // #PF

	//Iniciamos Scheduler
	iniciar_scheduler();

	/* Ejecutamos TODOS los módulos */
	if ( mbd->flags & 8 ) {
		module_t *mod;
		unsigned long i;
		for ( i = 0, mod = (module_t *) PA2KVA(mbd->mods_addr);
			i < mbd->mods_count;
			i++, mod++ )
			{
				programs_t *p = (programs_t *) mod->mod_start;
				if ( *((uint32_t *)p->magic) != 0x00455845 ) continue;
				crear_tarea( (programs_t *) mod->mod_start, i );
			}
	}

#if 0
	programs_t p1, p2;

	p1.va_entry = pruebatarea;
	p2.va_entry = pruebatarea2;

	crear_kthread( &p1, 0 );
	crear_kthread( &p2, 1 );
#endif

	set_irq_handler( 0, &timer );
	set_irq_handler( 1, &teclado );
	
	sti();

	for (;;) __asm__ __volatile__ ( "hlt" );
}

