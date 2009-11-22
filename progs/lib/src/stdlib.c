#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

void exit( int code ) {
	// De momento generamos una excepción.
	for ( ;; ) {
		__asm__ __volatile__ (
			"cli\n\t"
			"hlt\n\t"
		);
	}
}


static int last_random;
int rand() {
	last_random = 1103515245 * last_random + 12345;
	return 0x7fffffff & last_random; // Positivo siempre.
}

void srand( unsigned int semilla ) {
	last_random = semilla;
}

void *calloc( size_t n, size_t m ) {
	void *ptr = malloc ( n * m );
	memset( ptr, 0, n * m );
	return ptr;
}

void *malloc( size_t s ) {
	return NULL;
}

void free( void *ptr ) {
}

void *realloc( void *ptr, size_t n ) {
	// TODO
	return NULL;
}
