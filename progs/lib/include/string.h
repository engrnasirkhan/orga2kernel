#ifndef __STRING__H__
#define __STRING__H__

#include <config.h>
#include <types.h>
CDECLSTART

#define bzero(x,y) memset((x), 0, (y))

extern void *memset( void *s, int c, size_t n );
extern void *memcpy( void *d, void *o, size_t n );
// extern void *memmove( void *d, void *o, size_t n );

extern char *strcpy( char *d, const char *o );
extern char *strncpy( char *d, const char *o, int n );
extern size_t strlen( const char * );
// extern char *strchr( const char *s, int c );
// extern char *strrchr( const char *s, int c );

extern char *strcat( char *d, char *o );
extern char *strncat( char *d, char *o, size_t n );

CDECLEND
#endif // __STRING__H__
