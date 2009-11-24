#ifndef __STRING__H__
#define __STRING__H__

#define bzero(x,y) memset((x), 0, (y))

#include <asm/types.h>

void *memset( void *s, int c, size_t n );
void *memcpy( void *d, void *o, size_t n );

char *strcpy( char *d, const char *o );
char *strncpy( char *d, const char *o, int n );
size_t strlen( const char * );

char *strcat( char *d, char *o );
char *strncat( char *d, char *o, size_t n );

int strcmp( const char *s1, const char *s2 );
int strncmp( const char *s1, const char *s2, size_t n );

#endif // __STRING__H__
