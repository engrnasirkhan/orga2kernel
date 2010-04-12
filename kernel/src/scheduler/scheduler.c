#include <scheduler/scheduler.h>


#include <asm/types.h>
#include <asm/asm.h>
#include <asm/gdt.h>
#include <kernel/globals.h>
#include <boot/programs.h>
#include <mem/mmu.h>
#include <scheduler/tss.h>

#define limite_nueva_tss 0x67

void pruebaFuncion(){
 while(1) kprint(" Funciona ");	
}


//Funcion que muestra menu
void menu(){
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
	//Paso a la siguiente potencialmente ejecutable tarea
	do {
		tarea_activa = (tarea_activa + 1) % 10;	
	} while ( !tareas[tarea_activa].hay_tarea );
	
	//Chequeo indice en la gdt de la proxima tarea a ejecutar
	char gdt_indice = offset_gdt_tareas + tarea_activa;
	
	//Lanzo proxima tarea (de indice gdt_i en gdt) con RPL 11
	//uint16_t selector_prox = (gdt_indice << 3) | 3;
	uint16_t selector_prox = (gdt_indice << 3) | 0; // TODO: Usar el de arriba.
	
	kprint( "Saltando a tarea: %d, %d, %x\n", tarea_activa, gdt_indice, selector_prox );
	///TODO: Hacer lo de abajo, nose como.
	//__asm__ __volatile__ ("jmp selector_prox:00");
	//probando asm("jmp 0x1000");
	__asm__ __volatile__ (
		"pushl %0\n\t"
		"pushl $0\n\t"
		"ljmp *(%%esp)"
		: : "rm"(selector_prox)
	);
}

//Funcion para matar tarea
void matar_tarea(char numero_tarea){
	//Debe ejecutarse en contexto kernel
	cli();

//Voy a borrar las cosas que pueda en contexto de kernel
    kfree(tareas[numero_tarea].pantalla);
	kfree(tareas[numero_tarea].va_tss);


//Voy a poner en cero la estructura de la tarea
	tareas[numero_tarea].hay_tarea = 0;

//Voy a pasarle la direccion de la pdt a un funcion de mmu para que libera todo lo respecto a ella
	///TODO: faltaria funcion que le pase una pdt y borre todo lo referido a esta
	
	sti();
}


//Funcion para mostrar un slot en particular
void mostrar_slot(char s){
	//Funcion que pasa del buffer de pantalla de la tarea que se quiere mostrar, a la pantalla
	char * b_pantalla;
	b_pantalla = 0xb8000;
	for(unsigned int a=0; a< tam_buffer_pantalla; ++a) {
		b_pantalla[a]= (tareas[s]).pantalla[a];
	}
}


//Funcion para iniciar todo lo relativo al scheduler
void iniciar_scheduler(){

	// Creamos el TSS inicial para el kernel
	uint32_t virtual, fisica;
	reg_t seg;
	if ( mmu_alloc( PA2KVA(getCR3()), &virtual, PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR, &fisica ) == E_MMU_NO_MEMORY )
		panic( "No se pudo crear el TSS inicial." );
	gdt_fill_tss_segment( g_GDT + 5, virtual, 0x67, 0 ); 
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
	menu();
}


//Funcion para crear una nueva tarea
void crear_tarea(programs_t programa, char numero_tarea){
//Esta funcion va a tomar un programs_t, char con el numero donde lo quiere poner y apartir de ahi va a crear una tarea.
//Esta funcion se debe ejecutar siempre en el contexto del kernel

	cli();


//Si ya habia una tarea corriendo en ese slot, la mato ya la reemplazo
	if(tareas[numero_tarea].hay_tarea) matar_tarea(numero_tarea);

//Creamos nuevo directorio tabla de pagina
	
	uint32_t fisica_dtp;
    uint32_t virtual_dtp;
    if ((install_task_pdt(&virtual_dtp,&fisica_dtp)) == E_MMU_NO_MEMORY) kprint("Error al crear Tabla de Paginas ");


//Pido pagina para nueva tss (esto lo hago desde la pdt del kernel)
	uint32_t fisica_tss;
    uint32_t virtual_tss;
    uint8_t perm = 2; 
	if ((mmu_alloc( PA2KVA(getCR3()) , &virtual_tss, perm , &fisica_tss)) == E_MMU_NO_MEMORY) kprint("Error al crear TSS nueva tarea");
	tareas[numero_tarea].va_tss = virtual_tss;
	tareas[numero_tarea].pa_tss = fisica_tss;
	struct tss *nueva_tss = (struct tss *) virtual_tss;
	
//Pido pagina para codigo en el contexto de la nueva tarea y la mapeo a la pdt nueva
	///TODO: Hasta ahora solo agarra una pagina para el codigo, habria que agarrar las que hagan falta.
	uint32_t virtual_codigo;
	uint32_t fisica_codigo;
    if ((mmu_alloc((uint32_t *)virtual_dtp, &virtual_codigo, perm, &fisica_codigo)) == E_MMU_NO_MEMORY) kprint("Error pedir pag codigo nueva tarea");



//Copiar de donde estaba al codigo a la(s) nueva(s) pagina(s)
	///TODO: 
	/*
	 * Aca estaba probando algo q no va
	uint32_t *aFisicaDeprograma =0x10000000;
	
	if( (page_map_pa2va(PA2KVA(getCR3()) , fisica_codigo,aFisicaDeprograma,PAGE_USER|PAGE_RW|PAGE_PRESENT, 1))==E_MMU_NO_MEMORY) kprint("Error page_map_pa2va  1 ");
	
	uint32_t *pro = &(pruebaFuncion);
	for(int a=0; a<1024;a++) aFisicaDeprograma[a] = pro[a];
	*/

//Pido pagina para pila en el contexto de la nueva tarea y la mapeo a la pdt de la tarea 
	uint32_t virtual_pila;
	uint32_t fisica_pila;
	if ((mmu_alloc((uint32_t *)virtual_dtp, &virtual_pila, perm, &fisica_pila))== E_MMU_NO_MEMORY) kprint("Error pedir pag pila nueva tarea");


//Agrego una entrada nueva de GDT
	gdt_fill_tss_segment( &(g_GDT[numero_tarea+ offset_gdt_tareas]) , virtual_tss , limite_nueva_tss, 3);
 
//Lleno tss
	nueva_tss->cr3=	fisica_dtp;

/*#define USER_CS 0x1B
#define USER_DS 0x23*/
#define USER_CS 0x08
#define USER_DS 0x10
	nueva_tss->eip =  virtual_codigo;///TODO: Esto podria no ser cierto, podria tener otro entry point
	nueva_tss->eflags= 0x296; //Por poner alguno valido ( NOTE: IF ).
	nueva_tss->ebp= virtual_pila;
	nueva_tss->esp= virtual_pila;
	nueva_tss->cs= USER_CS;
	nueva_tss->ds= USER_DS;
	nueva_tss->es= USER_DS;
	nueva_tss->ss= USER_DS;
	nueva_tss->fs= USER_DS;
	nueva_tss->gs= USER_DS;
	
	
//Creo una pagina nueva, en el contexto del kernel, que va a funcionar como buffer de video para la nueva tarea
//Mapeo la direccion 0xb8000 de la tarea nueva, con la direccion fisica de la pagina que pedi recien
	
	//Pido pagina para buffer mapeado en kernel, la pido con mmu_alloc para tener tambien la fisica
	uint32_t virtual_video_kernel;
	uint32_t fisica_video_kernel;
	if ((mmu_alloc( PA2KVA(getCR3()), &virtual_video_kernel, perm, &fisica_video_kernel))== E_MMU_NO_MEMORY) kprint("Error pedir pag buffer video nueva tarea");
	
	
	//mapeo 0xb8000, con la direccion fisica de nuevo_buffer_video en nueva pdt
	if( (page_map_pa2va(virtual_dtp, fisica_video_kernel,0xb8000,PAGE_USER,1))!=E_MMU_SUCCESS) kprint("Error page_map_pa2va");
	
//Modifico las variables correspondientes al scheduler y tarea
	
	tareas[numero_tarea].hay_tarea= 1;
	tareas[numero_tarea].quantum_fijo = quantum_default;
	tareas[numero_tarea].quantum_actual = 0;
	tareas[numero_tarea].pantalla = virtual_video_kernel;


	if (tarea_activa < 0) scheduler();
	sti();
}


