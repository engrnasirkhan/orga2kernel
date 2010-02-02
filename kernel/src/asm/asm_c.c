#include <asm/types.h>

void lidt( void *idtr ) {
	idtr_t *idtreg = idtr;
	__asm__ __volatile__ ( "lidt %0" : : "m"(*idtreg) );
}

void setCR3( reg_t cr3 ) {
	__asm__ __volatile__ ( "movl %0, %%cr3" : : "r"(cr3) );
}

reg_t getCR3() {
	reg_t cr3;
	__asm__ __volatile__ ( "movl %%cr3, %0" : "=r"(cr3) );
	return cr3;
}

void setCR0( reg_t cr0 ) {
	__asm__ __volatile__ ( "movl %0, %%cr0" : : "r"(cr0) );
}

reg_t getCR0() {
	reg_t cr0;
	__asm__ __volatile__ ( "movl %%cr0, %0" : "=r"(cr0) );
	return cr0;
}

void setCR4( reg_t cr4 ) {
	__asm__ __volatile__ ( "movl %0, %%cr4" : : "r"(cr4) );
}

reg_t getCR4() {
	reg_t cr4;
	__asm__ __volatile__ ( "movl %%cr4, %0" : "=r"(cr4) );
	return cr4;
}

/*
void outb( uint16_t port, uint8_t value ) {
}

void outw( uint16_t port, uint16_t value ) {
}

void outd( uint16_t port, uint32_t value ) {
}

uint8_t inb( uint16_t port ) {
	__asm__ __volatile__
}

uint16_t inw( uint16_t port ) {
}

uint32_t ind( uint16_t port ) {
}

*/
