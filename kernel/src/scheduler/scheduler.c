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

#define limite_nueva_tss 0x67

//Funcion que muestra menu
void menu(key s){
	return; 
	//__asm__ __volatile__ ("xchg %bx,%bx");
kclrscreen();
kprint("Menu miOS: Como operar \n \n \n \n");
kprint("- Para cargar programa: cargar letra_de_programa numero_de_slot (1-10).\n");
kprint("        ej:  cargar c 4  Para cargar programa c en slot 4 \n \n");
kprint("- Para matar un programa: matar numero_de_slot \n");
kprint("        ej:  matar 5 \n \n");
kprint("- Para cambiar quantum: quantum numero_de_slot valor(1-20) \n");
kprint("        ej:  quantum numero_de_slot 13 \n \n \n\n \n \n \n \n");

// notar que cuando decimos numero de slot, tenemos que tener en cuenta
// que tarea i esima esta en el numero de slot i-esimo + 1
// Osea que si alguien quiere crear una tarea en el slot i, debemos pasarle a la funcion que las crea el numero de tarea i-1
}


//Funcion de scheduler
void scheduler(){	
	uint32_t i;
	//Paso a la siguiente potencialmente ejecutable tarea
	for ( i = 0; i < 10; i++ ) {
		tarea_activa = (tarea_activa + 1) % 10;
		if ( tareas[tarea_activa].hay_tarea ) break;
	}
	if ( !tareas[tarea_activa].hay_tarea ) {
		tarea_activa = -1;
		return;
	}
	
	//Chequeo indice en la gdt de la proxima tarea a ejecutar
	char gdt_indice = offset_gdt_tareas + tarea_activa;
	
	//Lanzo proxima tarea (de indice gdt_i en gdt) con RPL 11
	//uint16_t selector_prox = (gdt_indice << 3) | 3;
	uint16_t selector_prox = (gdt_indice << 3) | 0; // TODO: Usar el de arriba.

	kprint( "Saltando a tarea: %d, %d, %x\n", tarea_activa, gdt_indice, selector_prox );
	kprint( "CR3: %x\n", ((tss_t *) tareas[tarea_activa].va_tss)->cr3 );
	__asm__ __volatile__ ("xchg %bx,%bx" );
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

//Voy a pasarle la direccion de la pdt a un funcion de mmu para que libera todo lo respecto a ella
	///TODO: faltaria funcion que le pase una pdt y borre todo lo referido a esta
	
	unmask_ints(flags);
}


//Funcion para mostrar un slot en particular
void mostrar_slot(key s){
	//Funcion que pasa del buffer de pantalla de la tarea que se quiere mostrar, a la pantalla
	char * b_pantalla;
	b_pantalla = (char *) 0xb8000L;
	for(unsigned int a=0; a< tam_buffer_pantalla; ++a) {
		b_pantalla[a]= (tareas[s]).pantalla[a];
	}
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
	
	//Tarea en pantalla = menu (-1)
	tarea_en_pantalla = -1;
	
	//Tarea activa = -1
	tarea_activa = -1;
	
	//Pongo en 0 contador actualizar pantalla
	contador_actualizar_pantalla = 0;
	
	//Lanzamos programa menu
	menu(1);
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
	nueva_tss->esp = pila_va;
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
	uint32_t tss_va, tss_pa;
	uint32_t paginas, i;
	uint32_t va, pa;
	pde_t *pdt;
	struct tss *nueva_tss;

	if ( !programa || id < 0 ) return;

	/* ¿Es una tarea del kernel o del usuario? */
	if ( (uint32_t) programa->va_entry >= KERNEL_MEMMAP )

	if ( mmu_install_task_pdt( &pdt_va, &pdt_pa ) != E_MMU_SUCCESS ) {
		kprint( "No se pudo obtener memoria para el nuevo PDT.\n" );
		return;
	}
	pdt = (pde_t *) pdt_va;

	/* TODO: Necesitamos una función que obtenga una frame y lo marque como
	 * usado, pero que NO lo mapee en el espacio de usuario.
	 * TODO: No gastar una página entera para la TSS... usar kmalloc + alineación.
	 */
	if ( mmu_alloc( pdt, &tss_va, &tss_pa, PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR )
		!= E_MMU_SUCCESS ) {
		kprint( "Error al obtener una página para la TSS.\n" );
		return;
	}

	nueva_tss = (struct tss *) PA2KVA(tss_pa);

	/* El código no necesitamos copiarlo, pues es RO */
	va = (uint32_t) programa->va_text;
	pa = (uint32_t) KVA2PA(programa);
	paginas = (programa->va_data - programa->va_text) >> 12;
	if ( !paginas ) {
		kprint( "Información de programa errónea.\n" );
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
	 */
	paginas = programa->va_bss - programa->va_data;
	for ( i = 0; i < paginas; i++ ) {
		if ( mmu_alloc_at_VA( pdt, va, PAGE_PRESENT | PAGE_RW | PAGE_USER, 0 ) !=
			E_MMU_SUCCESS ) {
			kprint( "Error al obtener una página en la dirección virtual 0x%x\n", va );
			return;
		}

		/* Copiamos los datos. */
		memcpy( (void *)va, (void *)pa, PAGE_SIZE );
		pa += PAGE_SIZE;
		va += PAGE_SIZE;
	}

	/* Ahora hay que mapear la sección BSS, y llenarla de ceros. */
	paginas = programa->va_bssend - programa->va_bss;
	for ( i = 0; i < paginas; i++ ) {
		if ( mmu_alloc_at_VA( pdt, va, PAGE_PRESENT | PAGE_RW | PAGE_USER, 0 ) !=
			E_MMU_SUCCESS ) {
			kprint( "Error al obtener una página en la dirección virtual 0x%x\n", va );
			return;
		}
		memset( (void *)va, 0, PAGE_SIZE );
		va += PAGE_SIZE;
	}

	/* Ahora pedimos un espacio cualquier para la pila. */
	if ( mmu_alloc( pdt, &va, &pa, PAGE_PRESENT | PAGE_RW | PAGE_USER ) != E_MMU_SUCCESS ) {
		kprint( "Error al pedir pila para el proceso.\n" );
		return;
	}

	memset( nueva_tss, 0, sizeof( struct tss ) );
	nueva_tss->cr3 = pdt_pa;
	nueva_tss->eflags = USER_FLAGS; //0x296;
	nueva_tss->eip = (uint32_t) programa->va_entry;
	nueva_tss->esp = va;
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
	tareas[id].va_tss = (void *) PA2KVA(tss_pa);
	tareas[id].pa_tss = (void *) tss_pa;
	tareas[id].quantum_fijo = quantum_default;
	tareas[id].quantum_actual = 0;

	/* Ya mapeamos todo, ahora construimos la TSS :-) */
	gdt_fill_tss_segment( g_GDT + id + offset_gdt_tareas, (void *) PA2KVA(tss_pa), 0x67, 0 );

	unmask_ints(flags);
}
