#ifndef __TYPES__H__
#define __TYPES__H__

/* valores */
#define NULL 0

/* Tipos signados */
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;

/* Tipos no signados */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

/* bool type */
typedef signed int bool;
#define true 1;
#define false 0;

typedef uint32_t reg_t;
typedef void *ptr_t;
typedef unsigned int size_t;
typedef signed int time_t;

typedef struct __attribute__ ((packed)) {
	uint16_t limit;
	uint32_t base;
} gdtr_t; // __attribute__ ((packed));
typedef gdtr_t idtr_t;

struct registers {
	reg_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Guardados por PUSHA.
	uint32_t nro, errcode; // Pusheadas a mano.
	reg_t eip, cs, eflags; // Pusheadas por la interrupción.
	reg_t old_esp, ss; // Pusheadas sólo en cambio de privilegio (cs != kernel_cs)
} __attribute__ ((packed));

typedef struct __attribute__((packed)) registers registers_t;

typedef int (*handler_t)( struct registers * );
//typedef (int (*) (struct registers *)) handler_t;


#endif
