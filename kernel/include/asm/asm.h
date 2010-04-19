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

#define USER_FLAGS 0x202
#define KERNEL_FLAGS 0x202

/** @brief Carga el registro GDTR con la información suministrada. */
extern void lgdt( void *gdtr );
/*void  lgdt( void *gdtr ) {
	__asm__ __volatile__ ( "lgdt %0" : : "m"(gdtr) );
}*/

/** @brief Carga el registro IDTR con la información suministrada. */
extern void lidt( void *idtr );

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

extern reg_t getCR2();

/** @breif Invalida la TLB para la direccion addr*/
extern void invlpg(uint32_t addr);

/* Entrada y salida de puertos */
/*
extern void outb( uint16_t port, uint8_t value );
extern void outw( uint16_t port, uint16_t value );
extern void outd( uint16_t port, uint32_t value );
extern uint8_t inb( uint16_t port );
extern uint16_t inw( uint16_t port );
extern uint32_t ind( uint16_t port );
*/
extern void io_wait();

static inline uint8_t inb( uint16_t port ) {
	uint8_t res;
	__asm__ __volatile__ ( "inb %%dx, %%al" : "=al"(res) : "dx"(port) );
	return res;
}

#define sti() __asm__ __volatile__( "sti" );
#define cli() __asm__ __volatile__( "cli" );
#define hlt() __asm__ __volatile__( "hlt" );
#define outb(p,v) __asm__ __volatile__("outb %%al, %%dx" : : "al"(v), "dx"(p) )
#define outw(p,v) __asm__ __volatile__("outw %%al, %%dx" : : "ax"(v), "dx"(p) )
#define outd(p,v) __asm__ __volatile__("outd %%al, %%dx" : : "eax"(v), "dx"(p) )

#define mask_ints(f) __asm__ __volatile__( "pushfl\n\tpopl %0\n\tcli" : "=rm"(f) )
#define unmask_ints(f) __asm__ __volatile__( "pushl %0\n\tpopfl" : : "rm"(f) )

#endif // __ASM__H__
