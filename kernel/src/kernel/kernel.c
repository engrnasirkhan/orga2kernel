#include <asm/asm.h>
#include <asm/types.h>
#include <asm/handlers.h>
#include <boot/multiboot.h>
#include <kernel/panic.h>
#include <screen/screen.h>
#include <asm/gdt.h>
#include <asm/idt.h>
#include <asm/handlers.h>
#include <asm/syscalls.h>
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
tty_t tty_kernel;
/* ARG1 = fd de salida.
 * ARG2 = puntero (en espacio de usuario).
 * ARG3 = tamaño.
 */
int sys_write( struct registers *r ) {
	int i;
	const char *str = (const char *) ARG2(r);
	if ( ARG1(r) != 1 ) return -1;


	/* if ( chequear_direccion(r->ecx, r->edx) == MALA ) return -1; */
	for ( i = 0; i < ARG3(r); i++ )
		kputc( str[i] );
	return ARG3(r);
}

int sys_getpid() {
	return tarea_activa;
}

int sys_exit( struct registers *r ) {
	cli();
	matar_tarea( tarea_activa );
	panic_regs( r, "Falló sys_exit!!" );
	return 0; /* No retorna!! */
}

/* Nota, hay que mandarle el EOI al PIC porque scheduler() NO vuelve,
 * Si no le mandamos el EOI nosotros no se lo manda nadie.
 * Además, hay que mandarle el EOI antes de hacer el cambio de contexto,
 * si lo hacemos después lo vamos a estar haciendo dos veces.
 * Una vez cuando nos vuelve a tocar la ejecución y volvemos de scheduler()
 * y la otra cuando volvemos al despachador de interrupciones.
 */
int timer( struct registers *r ) {
	#define TIEMPO_ACTUALIZCION 10  //Totalmente arbitrario
	if (tarea_activa == -1) {
		pic8259A_send_EOI( r->nro );
		scheduler();
	}
		
	//Referente a la actualizacion de pantalla activa
	/*++contador_actualizar_pantalla;
	if(contador_actualizar_pantalla > TIEMPO_ACTUALIZCION) { 
		contador_actualizar_pantalla = 0;
		if( tarea_en_pantalla != -1 )mostrar_slot(tarea_en_pantalla+1);
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

static void idletask() {
	while(1);
}

static int pf( struct registers *r ) {
	cli();

	uint32_t cr2 = getCR2();
	pde_t *cr3 = (pde_t *) PA2KVA(GET_BASE_ADDRESS(getCR3()));
	pte_t *pte = (pte_t *) PA2KVA(GET_BASE_ADDRESS(cr3[ GET_PD_OFFSET(cr2) ]));

	kprint( r->errcode & 1 ? "Page level protection\n" : "Page not present\n" );	
	kprint( r->errcode & 2 ? "Write error\n" : "Read error\n" );
	kprint( r->errcode & 4 ? "User mode\n" : "Supervisor mode\n" );	
	kprint( r->errcode & 8 ? "Reserved bits en 1 en PD\n" : "Reserved bits en 0 en PD\n" );	
	kprint( r->errcode & 16 ? "Instruction Fetch\n" : "No fue Instruction Fetch\n" );	
	kprint( "CR2: 0x%x\nDir: 0x%x:0x%x\n", cr2, r->cs, r->eip );

	if ( r->cs == USER_CS ) {
		kprint( "Matando tarea %d\n", tarea_activa );
		matar_tarea( tarea_activa );
	}

	/*if ( cr2 < KERNEL_MEMMAP ) {
		kprint ( "PDE Flags: %x\n", cr3[ GET_PD_OFFSET(cr2) ] & 0xF );
		kprint ( "PTE Flags: %x\n", pte[ GET_PT_OFFSET(cr2) ] & 0xF );
		if ( r->errcode & 1 ) { // Si estaba presente le arreglo esto.
			kprint ( "Arreglando.\n" );
			cr3[ GET_PD_OFFSET(cr2) ] |= PAGE_USER | PAGE_RW;
			invlpg( GET_BASE_ADDRESS(cr2) );
			return 0;
		}
	}*/
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

    kprint("llego\n");
    key_init();
    
    //Aca asigna Esc a la funcion menu
    key_register(menu, 1);
    //Seteo Fs
    key_register(mostrar_slot,59);
    key_register(mostrar_slot,60);
    key_register(mostrar_slot,61);
    key_register(mostrar_slot,62);
    key_register(mostrar_slot,63);
    key_register(mostrar_slot,64);
    key_register(mostrar_slot,65);
    key_register(mostrar_slot,66);
    key_register(mostrar_slot,67);
    
    
    
    
    kprint("binding keys done\n");
    if(tty_init(&tty_kernel, menu_in)){
        panic("fallo  el inicio de las tty");
    }
	//Lanzamos programa para cargar tareas y modificar quantums.
	set_isr_handler( 14, &pf ); // #PF

	// El idle task... siempre listo... :-)
	programs_t idle;
	idle.va_entry = idletask;
	crear_kthread( &idle, 10 );

	//Iniciamos Scheduler
	iniciar_scheduler();

	extern unsigned char ej1[];
	extern unsigned char ej2[];
	extern unsigned char ej3[];
	extern unsigned char ej4[];
	programas[0] = (programs_t *) ej1;
	programas[1]= (programs_t *) ej2;
	programas[2] = (programs_t *) ej3;
	programas[3]= (programs_t *) ej4;
	programas[4]= (programs_t *) ej1;
	
	set_irq_handler( 0, &timer );
	set_irq_handler( 1, &irq_keyboard );
   menu(1);
	sti();
    //Lanzamos programa menu
	while(1);
}

