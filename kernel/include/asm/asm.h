#ifndef __ASM__H__
#define __ASM__H__

#include <asm/types.h>

/** @file asm.h
 *  
 *  Contiene funciones de utilidad de bajo nivel.
 */

#define __init __attribute__ ((section (".init.text")))
#define __init_rodata __attribute__ ((section (".init.rodata")))
#define __init_data __attribute__ ((section (".init.data")))
#define __init_bss __attribute__ ((section (".init.bss")))

/** @brief Carga el registro GDTR con la información suministrada. */
void inline lgdt( void *gdtr ) {
	__asm__ __volatile__ ( "lgdt %0" : : "m"(gdtr) );
}

/** @brief Carga el registro IDTR con la información suministrada. */
void inline lidt( void *idtr ) {
	__asm__ __volatile__ ( "lidt %0" : : "m"(idtr) );
}

/** @brief Carga el registro IDTR con la información suministrada. */
void inline setCR3( reg_t cr3 ) {
	__asm__ __volatile__ ( "movl %0, %%cr3" : : "r"(cr3) );
}

/** @brief Obtiene el contenido del CR3 */
reg_t getCR3() {
	reg_t cr3;
	__asm__ __volatile__ ( "movl %%cr3, %0" : "=r"(cr3) );
	return cr3;
}

/** @brief Establece el contenido del CR0 */
void inline setCR0( reg_t cr0 ) {
	__asm__ __volatile__ ( "movl %0, %%cr0" : : "r"(cr0) );
}

/** @brief Obtiene el contenido del CR0 */
reg_t getCR0() {
	reg_t cr0;
	__asm__ __volatile__ ( "movl %%cr0, %0" : "=r"(cr0) );
	return cr0;
}

/** @brief Establece el contenido del CR4 */
void inline setCR4( reg_t cr4 ) {
	__asm__ __volatile__ ( "movl %0, %%cr0" : : "r"(cr0) );
}

/** @brief Obtiene el contenido del CR4 */
reg_t getCR4() {
	reg_t cr4;
	__asm__ __volatile__ ( "movl %%cr4, %0" : "=r"(cr4) );
	return cr4;
}

#endif // __ASM__H__
