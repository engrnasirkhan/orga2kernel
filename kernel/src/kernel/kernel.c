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
static reg_t mapa_programa[1024] __attribute__ ((aligned (4096)));

static void ejecutar ( unsigned long phys_start, unsigned long phys_end, char *cmdline ) {
	programs_t *p = (programs_t *) phys_start;
	unsigned int *cr3;
	reg_t entrada_vieja;
	int ret_status;

	if ( p->magic[0] != 'E' ||
		p->magic[1] != 'X' ||
		p->magic[2] != 'E' ||
		p->magic[3] != 0 ) {
		kprint( "Modulo no reconocido :-(\n" );
		return;
	}

	// Llenamos de cero el mapa del programa.
	for ( int i = 0; i < 1024; i++ )
		mapa_programa[i] = 0;
	
	// Páginas de código.
	int paginas = ((unsigned int) p->va_data - (unsigned int) p->va_text) >> 12;
	int i;
	for ( i = 0; i < paginas; i++ )
		mapa_programa[i] = (phys_start + 4096 * i) | 5; // User, Read-only, present
	
	// Páginas de datos.
	paginas = (unsigned int) p->va_bssend;
	paginas = (paginas + 4095) & ~4095; // Redondeamos al próximo múltiplo de página (4KB).
	paginas -= (unsigned int) p->va_data;
	paginas >>= 12;
	paginas += i;
	for ( ; i < paginas; i++ )
		mapa_programa[i] = (phys_start + 4096 * i) | 7; // User, Read-Write, present

	// 1. Obtenemos cr3
	// 2. Establecemos la dirección física de la entrada adecuada.
	// 3. Invalidamos la página
	// 4. Ponemos en cero la sección bss.
	// 5. Saltamos a la función adecuada
	__asm__ __volatile__ ( "movl %%cr3, %0" : "=r"(cr3) );
	unsigned int *entrada = cr3 + (((unsigned int) p->va_text) >> 22);
	entrada_vieja = *entrada;
	*entrada = (((unsigned long) mapa_programa) - 0x80000000) | 3; // System, Read-Write, Present (puntero a tabla de páginas)

	// Podemos o invalidar la página o recargar todo cr3 :P
	//__asm__ __volatile__ ( "invlpg %0" : : "m"(*((unsigned int*)p->va_text)) );
	__asm__ __volatile__ ( "movl %0, %%cr3" : : "r"(cr3) );

	// Ponemos en cero bss (ahora que podemos acceder vía las direcciones virtuales :-)
	unsigned char *bss = p->va_bss;
	while ( bss != p->va_bssend )
		*bss++ = 0;

	__asm__ __volatile__ (
		"pushl %1\n\t"
		"call *%2\n\t"
		"addl $4, %%esp\n\t"
		"movl %%eax, %0"
		: "=rm"(ret_status) : "rm"(cmdline), "r"(p->va_entry)
	);

	// Volvamos a como estábamos
	*entrada = entrada_vieja;
	__asm__ __volatile__ ( "movl %0, %%cr3" : : "r"(cr3) );

	kprint ( "El programa devolvio el codigo de salida: %d\n", ret_status );
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
    // Ejecutemos módulo por módulo.
    if ( mbd->flags & 8 ) {
        module_t *mod;
        unsigned long i;

        for ( i = 0, mod = (module_t *) mbd->mods_addr;
		        i < mbd->mods_count;
		        i++, mod++ ) {
	        ejecutar( mod->mod_start, mod->mod_end, (char *) mod->string );
        }
    }  
    
#endif
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

#if 0
	/* Ejecutamos TODOS los módulos */
	if ( mbd->flags & 8 ) {
        module_t *mod;
        unsigned long i;
		for ( i = 0, mod = (module_t *) mbd->mods_addr;
			i < mbd->mods_count;
			i++, mod++ )
			{
				programs_t *p = (programs_t *) mod->mod_start;
				if ( (uint32_t)p->magic != 0x00455845 ) continue;
				crear_tarea( (programs_t *) mod->mod_start, i );
			}
	}
#else
	programs_t p1, p2;

	p1.va_entry = pruebatarea;
	p2.va_entry = pruebatarea2;

	crear_kthread( &p1, 0 );
	crear_kthread( &p2, 1 );
#endif

	set_irq_handler( 0, &timer );
	//set_irq_handler( 1, &teclado );
	
	sti();

	for (;;) __asm__ __volatile__ ( "hlt" );
}

