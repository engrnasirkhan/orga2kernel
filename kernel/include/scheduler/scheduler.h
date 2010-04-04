#ifndef __SCHEDULER__H__
#define __SCHEDULER__H__

#define offset_gdt_tareas 6
#define tam_buffer_pantalla 8000
#define quantum_default 1000;
#define gdt_address 0x1234  /// TODO: PONER CORRECTA

typedef struct {
	char hay_tarea;						//1 si hay una tarea mapeada
    unsigned int quantum_fijo;			//Cuantum que se renueva cada vez que vence el actual
	unsigned int quantum_actual;		//Cuantum que va decreciendo
	char *pantalla;						//Direccion virtual de pantalla, con respecto de Pdt Kernel
	void *va_tss;
	void *pa_tss;
	//La tarea tiene reservado un espacio en la gdt, la tarea i-esima esta en la entrada offset_gdt_tareas +i de la gdt
} tarea;


//Funcion que muestra menu
void menu();

//Funcion de scheduler
void scheduler();

//Funcion para matar tarea
void matar_tarea(char numero_tarea);


//Funcion para crear una nueva tarea
void crear_tarea(programs_t programa, char numero_tarea);

//Funcion para mostrar un slot en particular
void mostrar_slot(char s);

//Funcion para iniciar todo lo relativo al scheduler
void iniciar_scheduler();

#endif // __SCHEDULER__H__
