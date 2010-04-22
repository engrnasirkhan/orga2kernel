#ifndef __SYSCALLS__H__
#define __SYSCALLS__H__

#include <asm/types.h>

#define SYSCALL_EXIT   0
#define SYSCALL_WRITE  1
#define SYSCALL_GETPID 2

#define NR_SYSCALLS    3

/**
 * @file syscalls.h
 * @brief Llamdas al sistema.
 *
 * Las llamadas al sistema son todas del tipo @b handler_t,
 * los parámetros se pasan en los siguientes registros (en orden):
 * @li eax (el ID de la llamada al sistema).
 * @li ecx
 * @li edx
 * @li ebx
 * @li esi
 * @li edi
 * @li En la pila de usuario.
 * Los primeros 3 registros no necesitan ser salvados en una función
 * por la convención de C.
 */
#define SYSCALL_NUMBER(x) ((x)->eax)
#define ARG1(x) ((x)->ecx)
#define ARG2(x) ((x)->edx)
#define ARG3(x) ((x)->ebx)
#define ARG4(x) ((x)->esi)
#define ARG5(x) ((x)->edi)

extern void syscall_entry();
extern handler_t syscall_table[];

extern int sys_write( struct registers * );
extern int sys_getpid();
extern int sys_exit( struct registers * );

#endif
