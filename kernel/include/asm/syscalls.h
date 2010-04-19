#ifndef __SYSCALLS__H__
#define __SYSCALLS__H__

#include <asm/types.h>

#define SYSCALL_WRITE 1
#define NR_SYSCALLS 2

/**
 * @file syscalls.h
 * @brief Llamdas al sistema.
 *
 * Las llamadas al sistema son todas del tipo @b handler_t,
 * los parámetros se pasan en los siguientes registros (en orden):
 * @li eax (el ID de la llamada al sistema).
 * @li ebx
 * @li ecx
 * @li edx
 * @li esi
 * @li edi
 * @li En la pila de usuario.
 */

extern void *syscall_entry;
extern handler_t syscall_table[];

extern int sys_write( struct registers * );

#endif
