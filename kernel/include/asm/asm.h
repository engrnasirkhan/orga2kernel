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
#define __noreturn __attribute__ ((noreturn))

/** @brief Carga el registro GDTR con la información suministrada. */
extern void lgdt( void *gdtr );
/*void  lgdt( void *gdtr ) {
	__asm__ __volatile__ ( "lgdt %0" : : "m"(gdtr) );
}*/

/** @brief Carga el registro IDTR con la información suministrada. */
extern void  lidt( void *idtr );

/** @brief Establece el contenido del CR3 */
extern void setCR3( reg_t cr3 );

/** @brief Obtiene el contenido del CR3 */
extern reg_t getCR3();

/** @brief Establece el contenido del CR0 */
extern void setCR0( reg_t cr0 );

/** @brief Obtiene el contenido del CR0 */
extern reg_t getCR0();

/** @brief Establece el contenido del CR4 */
extern void setCR4( reg_t cr4 );

/** @brief Obtiene el contenido del CR4 */
extern reg_t getCR4();

#endif // __ASM__H__
