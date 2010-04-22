#include <scheduler/scheduler.h>
#include <asm/types.h>
#include <asm/asm.h>
#include <asm/gdt.h>
#include <kernel/globals.h>
#include <boot/programs.h>
#include <mem/mmu.h>
#include <mem/vmm.h>
#include <mem/memlayout.h>
#include <scheduler/tss.h>
#include <screen/screen.h>
#include <kernel/panic.h>
#include <drivers/keyboard.h>
#include <string.h>
#include <tty/tty.h>

#define limite_nueva_tss 0x67

extern tty_t tty_kernel;

//Funcion que muestra menu y parsea los comandos.
void menu(key s){
	if (s) tarea_en_pantalla = -1;
	//__asm__ __volatile__ ("xchg %bx,%bx");
    kclrscreen();
    kprint("Menu miOS: Como operar \n \n \n \n");
    kprint("- Para cargar programa: cargar letra_de_programa numero_de_slot (0-9) Tenemos 5 progras distintos (a,b,c,d,e).\n");
    kprint("        ej:  cargar a 4  (cargar programa  a  en slot 4)    \n \n");
    kprint("- Para matar un programa: matar numero_de_slot \n");
    kprint("        ej:  matar 5  (mata el programa que corria en slot 5) \n \n");
    kprint("- Para cambiar quantum: quantum numero_de_slot valor(0-9) \n");
    kprint("        ej:  quantum 3 2 (cambia el quantum del programa que corre en slot 3 a 2\n \n \n\n \n \n \n \n");
    tty_get_string(&tty_kernel);
    
   
}
void menu_in(uint8_t* c) {
		//kprint("FUNCION QUE PARSEA LA ENTRADA Y DECIDE QUE HACER(SCHEDULER.C)\n");
		tty_get_string(&tty_kernel);

uint8_t *entrada = (tty_tty_find(&tty_kernel)->buff);

uint8_t modo=3;
uint8_t letra_tarea;
uint8_t numero_slot;
uint8_t quantum;
//cargar => modo=0
//matar =>  modo=1
//quantum => modo=2

if( entrada[0]=='c' && entrada[1]=='a' && entrada[2]=='r' && entrada[3]=='g' && entrada[4]=='a'&& entrada[5]=='r' && entrada[6]==' ' && entrada[8]==' '  &&  (entrada[7]<'f') && (entrada[7]>='a') && ( entrada[9]<='9' ) && ( entrada[9]>='0' ) )	modo=0;
if( entrada[0]=='m' && entrada[1]=='a' && entrada[2]=='t' && entrada[3]=='a' && entrada[4]=='r' && entrada[5]==' ' && ( entrada[6]<='9' ) && ( entrada[6]>='0' ) ) modo=1;
if( entrada[0]=='q' && entrada[1]=='u' && entrada[2]=='a' && entrada[3]=='n' && entrada[4]=='t' && entrada[5]=='u' && entrada[6]=='m' && entrada[7]==' ' )	modo=2;


if(modo==0){
	//crear
	
	letra_tarea= entrada[7];
	numero_slot= entrada[9] - '0';	
	kprint( "Cargando el programa %c en el slot %d\n", letra_tarea, numero_slot );
	crear_tarea( programas[ letra_tarea - 'a'], numero_slot);
}

if(modo==1){
	//matar
	numero_slot= entrada[6] - '0';	 
	if(tareas[numero_slot].hay_tarea) {
		matar_tarea(numero_slot);
		kprint("Mato la tarea del slot ");
		kprint("%i \n", numero_slot+ '0');
	
	}
}

if(modo==2){
	//cambiar quantum
	numero_slot= entrada[8] - '0';	
	quantum= entrada[10] - '0';	
	if( 0< quantum && quantum <21){
		 tareas[numero_slot].quantum_fijo = (int) quantum;
		kprint( "Cambio el quantum del slot %d al valor %d\n", numero_slot, quantum );
	} else kprint("Quantum invalido\n");

}
if(!( modo==2 || modo==1|| modo==0))kprint("Ingreso algo mal\n");



		
}

//Funcion de scheduler
void scheduler(){	
	uint32_t i;
	uint32_t tarea_anterior = tarea_activa;
	//Paso a la siguiente potencialmente ejecutable tarea
	for ( i = 0; i < 10; i++ ) {
		tarea_activa = (tarea_activa + 1) % 10;
		if ( tareas[tarea_activa].hay_tarea ) break;
	}
	if ( !tareas[tarea_activa].hay_tarea ) {
		tarea_activa = -1;
		return;
	}

	if ( tarea_activa == tarea_anterior ) return;
	
	//Chequeo indice en la gdt de la proxima tarea a ejecutar
	char gdt_indice = offset_gdt_tareas + tarea_activa;
	
	//Lanzo proxima tarea (de indice gdt_i en gdt) con RPL 11
	//uint16_t selector_prox = (gdt_indice << 3) | 3;
	uint16_t selector_prox = (gdt_indice << 3) | 0; // TODO: Usar el de arriba.

	//__asm__ __volatile__ ("xchg %bx,%bx" );
	__asm__ __volatile__ (
		"pushl %0\n\t"
		"pushl $0\n\t"
		"ljmp *(%%esp)\n\t"
		"addl $8, %%esp"
		: : "rm"(selector_prox)
	);
}

//Funcion para matar tarea
//Debe ejecutarse en contexto kernel
void matar_tarea(char numero_tarea){
	reg_t flags;
	mask_ints(flags);

//Voy a borrar las cosas que pueda en contexto de kernel
    kfree(tareas[numero_tarea].pantalla);
	kfree(tareas[numero_tarea].va_tss);


//Voy a poner en cero la estructura de la tarea
	tareas[numero_tarea].hay_tarea = 0;
	tareas[numero_tarea].quantum_fijo = 0;
	tareas[numero_tarea].pantalla = 0;
	

//Voy a pasarle la direccion de la pdt a un funcion de mmu para que libera todo lo respecto a ella
	///TODO: faltaria funcion que le pase una pdt y borre todo lo referido a esta
	
	unmask_ints(flags);
}


//Funcion para mostrar un slot en particular
void mostrar_slot(key s){
tarea_en_pantalla= s-1;
if( tareas[s-58].hay_tarea) memcpy( (void *) 0x800b8000, ( void * ) tareas[s-58].pantalla, 80 * 25 * 2 );
else menu(1);

}


//Funcion para iniciar todo lo relativo al scheduler
void iniciar_scheduler(){
	//Creamos el TSS inicial para el kernel
	uint32_t virtual, fisica;
	reg_t seg;
	if ( mmu_alloc( (pde_t *) PA2KVA(getCR3()), &virtual, &fisica, PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR ) == E_MMU_NO_MEMORY )
		panic( "No se pudo crear el TSS inicial." );

	gdt_fill_tss_segment( g_GDT + 5, (void *) virtual, 0x67, 0 ); 
	seg = 5<<3;
	__asm__ __volatile__ (
		"movw $5<<3, %%ax\n\t"
		"ltrw %%ax"
		: : : "ax" );
	
	//Pongo como no hay_tarea, para cada tarea
	for(char i=0; i<10; ++i) tareas[i].hay_tarea = 0;
	for(char i=0; i<10; ++i) tareas[i].quantum_fijo = 0;
	
	//Tarea en pantalla = menu (-1)
	tarea_en_pantalla = -1;
	
	//Tarea activa = -1
	tarea_activa = -1;
	
	//Pongo en 0 contador actualizar pantalla
	contador_actualizar_pantalla = 0;
	
}


/**
 * @brief Crea un kernel thread.
 *
 * A partir de los datos en @b programa genera el nuevo proceso. No crea
 * ningún mapa de memoria, ya que cualquier le viene bien (pues se ejecuta
 * en el contexto del kernel).
 */
void crear_kthread( programs_t *programa, char id ) {
	reg_t flags;
	uint32_t tss_va, pila_va;
	struct tss *nueva_tss;

	if ( !programa || id < 0 ) return;
	
	/* TODO: No gastar una página entera para la TSS... usar kmalloc + alineación. */
	if ( mmu_kalloc( &tss_va ) != E_MMU_SUCCESS ) {
		kprint( "Error al obtener una página para la TSS.\n" );
		return;
	}

	if ( mmu_kalloc( &pila_va ) != E_MMU_SUCCESS ) {
		kprint( "Error al obtener pila para el thread del kernel.\n" );
		return;
	}

	nueva_tss = (struct tss *) tss_va;
	memset( nueva_tss, 0, sizeof( struct tss ) );
	nueva_tss->cr3 = getCR3();
	nueva_tss->eflags = KERNEL_FLAGS;
	nueva_tss->eip = (uint32_t) programa->va_entry;
	nueva_tss->esp = pila_va + PAGE_SIZE;
	nueva_tss->cs = KERNEL_CS;
	nueva_tss->ds = KERNEL_DS;
	nueva_tss->es = KERNEL_DS;
	nueva_tss->ss = KERNEL_DS;
	nueva_tss->fs = KERNEL_DS;
	nueva_tss->gs = KERNEL_DS;

	/* Sección crítica... nos metemos con las tareas. */
	mask_ints(flags);
	if ( tareas[id].hay_tarea ) matar_tarea( id );

	tareas[id].hay_tarea = 1;
	tareas[id].va_tss = (void *) tss_va;
	tareas[id].pa_tss = (void *) KVA2PA(tss_va);
	tareas[id].quantum_fijo = quantum_default;
	tareas[id].quantum_actual = 0;

	/* Ya mapeamos todo, ahora construimos la TSS :-) */
	gdt_fill_tss_segment( g_GDT + id + offset_gdt_tareas, (void *) tss_va, 0x67, 0 );

	unmask_ints(flags);
}

/**
 * @brief Crea un nuevo proceso.
 *
 * A partir de los datos almacenados en @b programa genera
 * el espacio de direcciones del nuevo proceso.
 *
 * @param[in] programa El mapa de memoria del proceso.
 * @param[in] id Indica el número de slot a utilizar por la tarea.
 */
void crear_tarea( programs_t *programa, char id ) {
	reg_t flags;
	uint32_t pdt_va, pdt_pa;
	uint32_t tss_va;
	uint32_t paginas, i;
	uint32_t stack3_va, stack3_pa;
	uint32_t stack0_va, stack0_pa;
	uint32_t va, pa;
	uint32_t oldcr3;
	pde_t *pdt;
	struct tss *nueva_tss;

	if ( !programa || id < 0 ) return;

	if ( strncmp( programa->magic, "EXE", 4 ) ) {
		kprint( "No es un ejecutable reconocido.\n" );
		return;
	}

	/* ¿Es una tarea del kernel o del usuario? */
	if ( (uint32_t) programa->va_entry >= KERNEL_MEMMAP )
		return crear_kthread( programa, id );

	if ( mmu_install_task_pdt( &pdt_va, &pdt_pa ) != E_MMU_SUCCESS ) {
		kprint( "No se pudo obtener memoria para el nuevo PDT.\n" );
		return;
	}
	pdt = (pde_t *) pdt_va;

	/* TODO: No gastar una página entera para la TSS... usar kmalloc + alineación. */
	if ( mmu_kalloc( &tss_va ) != E_MMU_SUCCESS ) {
		kprint( "Error al obtener una página para la TSS.\n" );
		return;
	}

	nueva_tss = (struct tss *) tss_va;

	/* El código no necesitamos copiarlo, pues es RO */
	va = (uint32_t) programa->va_text;
	pa = (uint32_t) KVA2PA(programa);
	paginas = (programa->va_data - programa->va_text) >> 12;
	if ( !paginas ) {
		kprint( "Información de programa errónea.\n"
			"TEXT: 0x%x\n"
			"DATA: 0x%x\n"
			"BSS: 0x%x\n"
			"ENTRY: 0x%x\n",
			programa->va_text, programa->va_data,
			programa->va_bss, programa->va_entry
		);
		return;
	}
	for ( i = 0; i < paginas; i++ ) {
		if ( mmu_map_pa2va( pdt, pa, va, PAGE_PRESENT | PAGE_READONLY | PAGE_USER, 0 )
			!= E_MMU_SUCCESS ) {
			kprint( "Error al mapear 0x%x en 0x%x\n", pa, va );
			return;
		}
		pa += PAGE_SIZE;
		va += PAGE_SIZE;
	}

	/* Hasta acá ya tenemos mapeado el código, hay que hacer lo mismo con los datos.
	 * Hay que tener en cuenta que los datos sí hay que copiarlos.
	 * La sección .BSS y los datos pueden compartir la misma página, hay que tener
	 * cuidado con eso.
	 */
	paginas = ((programa->va_bssend - programa->va_data) + (PAGE_SIZE-1)) >> 12;
	for ( i = 0; i < paginas; i++ ) {
		if ( mmu_alloc_at_VA( pdt, va, PAGE_PRESENT | PAGE_RW | PAGE_USER, 0 ) !=
			E_MMU_SUCCESS ) {
			kprint( "Error al obtener una página en la dirección virtual 0x%x\n", va );
			return;
		}
		va += PAGE_SIZE;
	}

	// Pido pagina para buffer mapeado en kernel, la pido con mmu_alloc para tener tambien la fisica
	uint32_t virtual_video_kernel;
	if ( mmu_kalloc( &virtual_video_kernel ) != E_MMU_SUCCESS ) {
		kprint( "Error al pedir buffer de video.\n" );
		return;
	}
	if ( mmu_map_pa2va( (pde_t *) pdt_va, KVA2PA( virtual_video_kernel ), 0xb8000, PAGE_USER | PAGE_RW | PAGE_PRESENT, 0 ) != E_MMU_SUCCESS ) {
	//if ( mmu_map_pa2va( (pde_t *) pdt_va, 0xb8000, 0xb8000, PAGE_USER | PAGE_RW | PAGE_PRESENT, 0 ) != E_MMU_SUCCESS ) {
		kprint( "Error al mapear buffer de video.\n" );
		return;
	}

	/* Ahora que mapeamos las páginas de datos y de BSS hay que rellenarlas */
	oldcr3 = getCR3();
	setCR3( pdt_pa );
	memcpy( (void *) programa->va_data, (void *) PA2KVA(pa), programa->va_bss - programa->va_data );
	memset( (void *) programa->va_bss, 0, programa->va_bssend - programa->va_bss );
	memset( (void *) 0xB8000L, 0, 80 * 25 * 2 );
	setCR3( oldcr3 );

	/* Ahora pedimos un espacio cualquiera para la pila de usuario. */
	if ( mmu_alloc( pdt, &stack3_va, &stack3_pa, PAGE_PRESENT | PAGE_RW | PAGE_USER ) != E_MMU_SUCCESS ) {
		kprint( "Error al pedir pila para el proceso.\n" );
		return;
	}

	/* Y otro para la pila de kernel. */
	if ( mmu_alloc( pdt, &stack0_va, &stack0_pa, PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR ) != E_MMU_SUCCESS ) {
		kprint( "Error al pedir pila para el proceso.\n" );
		return;
	}

	memset( nueva_tss, 0, sizeof( struct tss ) );
	nueva_tss->cr3 = pdt_pa;
	nueva_tss->eflags = USER_FLAGS; //0x296;
	nueva_tss->eip = (uint32_t) programa->va_entry;
	nueva_tss->esp = stack3_va + PAGE_SIZE;
	nueva_tss->ss0 = KERNEL_DS;
	nueva_tss->esp0 = stack0_va + PAGE_SIZE;
	nueva_tss->cs = USER_CS;
	nueva_tss->ds = USER_DS;
	nueva_tss->es = USER_DS;
	nueva_tss->ss = USER_DS;
	nueva_tss->fs = USER_DS;
	nueva_tss->gs = USER_DS;

	/* Ahora viene la sección crítica... cuando nos metemos con las tareas. */
	mask_ints(flags);
	if ( tareas[id].hay_tarea ) matar_tarea( id );

	tareas[id].hay_tarea = 1;
	tareas[id].va_tss = (void *) tss_va;
	tareas[id].pa_tss = (void *) KVA2PA(tss_va);
	tareas[id].quantum_fijo = quantum_default;
	tareas[id].quantum_actual = 0;
	tareas[id].pantalla = virtual_video_kernel;

	/* Ya mapeamos todo, ahora construimos la TSS :-) */
	gdt_fill_tss_segment( g_GDT + id + offset_gdt_tareas, (void *) tss_va, 0x67, 0 );

	unmask_ints(flags);
}
