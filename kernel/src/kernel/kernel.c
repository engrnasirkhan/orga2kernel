#include <asm/asm.h>
#include <asm/types.h>
#include <boot/multiboot.h>
#include <boot/programs.h>
#include <screen/screen.h>
#include <asm/gdt.h>
#include <asm/idt.h>
#include <asm/handlers.h>
#include <kernel/globals.h>
#include <scheduler/scheduler.h>
#include <mem/memlayout.h>

//funcion que inicializa gran parte de las estructuras del kernel
extern void kinit ( multiboot_info_t* mbd ) __init;

static reg_t mapa_programa[1024] __attribute__ ((aligned (4096)));
//Creamos array de 10 tareas.
tarea tareas[10];
char tarea_activa;
char slot[10];

//funcion que imprime el menu
void menu(){
	//__asm__ __volatile__ ("xchg %bx,%bx");
kclrscreen();
kprint("Menu miOS: Como operar \n \n \n \n");
kprint("- Para cargar programa: cargar letra_de_programa numero_de_slot \n");
kprint("        ej:  cargar c 4  Para cargar programa c en slot 4 \n \n");
kprint("- Para matar un programa: matar numero_de_slot \n");
kprint("        ej:  matar 5 \n \n");
kprint("- Para cambiar quantum: quantum numero_de_slot valor(1-20) \n");
kprint("        ej:  quantum numero_de_slot 13 \n \n \n");
//kprint("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}

// funcion que permuta tareas
void scheduler(){
	tarea_activa= ((tarea_activa+1 )% 10);
	while(!(slot[(tarea_activa)% 10]))tarea_activa= ((tarea_activa+1 )% 10);  //vamos a la siguiente tarea si estaba vacia
	
	unsigned char gdt_i = (tareas[((tarea_activa)% 10)]).indice_gdt; // Chequeamos en que indice en la gdt esta la proxima tarea
	
	// lanzar_tarea ( gdt_i)

	
}


void mostrar_slot(char s){
	char * b_pantalla;
	b_pantalla = 0xb8000;
	for(unsigned int a=0; a< 4000; ++a) {
		b_pantalla[a]= (tareas[tarea_activa]).pantalla[a];
		}
}

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

int timer( struct registers *r ) {
	
	--(tareas[tarea_activa].quantum_actual); 			//decrementamos quantum
	if (tareas[tarea_activa].quantum_actual==0){		//si termino, reestablecemos y cambiamos a la proxima llamando a scheduler
			tareas[tarea_activa].quantum_actual = tareas[tarea_activa].quantum_fijo; // restablecemos quantums
			scheduler();// llamamos al scheduler
	}

	return 0;
}

int teclado( struct registers *r ) {

	cli();
	uint8_t key = inb( 0x60 );
//  __asm__ __volatile__ ("xchg %bx,%bx");
if (key==1 || (key >58 && key<69)){				//si entro aca fue para cambiar de slot o al menu
	if (key ==1) menu(); // Si apreto Esc
	if (key==59) mostrar_slot(1);
	if (key==60) mostrar_slot(2);
	if (key==61) mostrar_slot(3);
	if (key==62) mostrar_slot(4);
	if (key==63) mostrar_slot(5);
	if (key==64) mostrar_slot(6);
	if (key==65) mostrar_slot(7);
	if (key==66) mostrar_slot(8);
	if (key==67) mostrar_slot(9);
	if (key==68) mostrar_slot(10);
}else {}								// aca deberis ir algo para que capture lo que se escribe y lo mande en el slot donde esta o menu

    
	sti();
	return 0;
}

void kmain(multiboot_info_t*, unsigned int magic ) __noreturn;
void kmain(multiboot_info_t* mbd, unsigned int magic )
{
    if ( magic != 0x2BADB002 ) {
		// TODO: Hacer un kputs.
		kprint ( "Warning: No se inicializo con GRUB, intentando de todas formas.\n" );
	}

    //Primero tenemos que corregir mbd para que sea una direccion virtual valida
    mbd = (multiboot_info_t*)( PA2KVA( (uint32_t) mbd) );
    
    //Ahora si, usando mbd que apunta bien a los datos del grub, inicializamos todo
    kinit( mbd );

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

	//Inicializamos un par de cosas del scheduler que habria que mover a algun lado
	for (unsigned int i = 0; i <10 ; ++i) slot[i]=0;
	tarea_activa =0;
	
	//Lanzamos programa para cargar tareas y modificar quantums.
	menu();
	

	set_irq_handler( 0, &timer );
	set_irq_handler( 1, &teclado );
	
	sti();

	while (1);
}
