#ifndef __SCHEDULER__H__
#define __SCHEDULER__H__

#include <asm/types.h>
#include <asm/asm.h>
#include <asm/gdt.h>
#include <scheduler/tss.h>
#include <boot/programs.h>

#define offset_gdt_tareas 10
#define tam_buffer_pantalla 8000
#define quantum_default 1000;

typedef struct {
	char hay_tarea;						//1 si hay una tarea mapeada
    unsigned int quantum_fijo;			//Cuantum que se renueva cada vez que vence el actual
	unsigned int quantum_actual;		//Cuantum que va decreciendo
	char *pantalla;						//Direccion virtual de pantalla, con respecto de Pdt Kernel
	void *va_tss;
	void *pa_tss;
	//La tarea tiene reservado un espacio en la gdt, la tarea i-esima esta en la entrada offset_gdt_tareas +i de la gdt
} tarea;


//VARIABLES GLOBALES RELATIVAS A SCHEDULER
tarea tareas[10];					//Se almacena informacion referente a cada caterea
char tarea_activa;					//Numero de tarea en ejecucion (0-9), para no tarea -1
char tarea_en_pantalla;				//Numero de tarea mostrada por pantalla (0-9), para no tarea -1      
char contador_actualizar_pantalla;	//Contador que sirve para actualizar buffer de pantalla




//Funcion que muestra menu
void menu();

//Funcion de scheduler
void scheduler();

//Funcion para matar tarea
void matar_tarea(char numero_tarea);




//Funcion para mostrar un slot en particular
void mostrar_slot(char s);

//Funcion para iniciar todo lo relativo al scheduler
void iniciar_scheduler();


//Funcion para crear una nueva tarea
void crear_tarea(programs_t programa, char numero_tarea);

#endif // __SCHEDULER__H__
