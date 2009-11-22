#ifndef __STDLIB__H__
#define __STDLIB__H__

#include <config.h>
#include <types.h>
CDECLSTART

extern void exit( int code );
extern int rand();
extern void srand( unsigned int semilla );

extern void *calloc( size_t, size_t ); // Pone la memoria en cero!!!
extern void *malloc( size_t );
extern void free( void * );
extern void *realloc( void *, size_t );

CDECLEND
#endif // __STDLIB__H__
