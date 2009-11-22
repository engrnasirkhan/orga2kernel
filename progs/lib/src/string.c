#include <types.h>

void *memset( void *s, int c, size_t n ) {
	char *d = s;
	if ( !s ) return s;
	
	while ( n-- ) *d++ = (char) c;

	return s;
}

void *memcpy( void *d, void *o, size_t n ) {
	char *dc = d, *oc = o;
	if ( !d || !o ) return d;

	while ( n-- ) *dc++ = *oc++;

	return d;
}

char *strcpy( char *d, const char *o ) {
	char *res = d;
	if ( !d || !o ) return res;
	while ( *o ) *d++ = *o++;
	*d = 0;
	return res;
}

char *strncpy( char *d, const char *o, int n );

size_t strlen( const char *str ) {
	size_t res = 0;
	if ( !str ) return 0;
	while ( *str++ ) res++;
	return res;
}

char *strcat( char *d, char *o ) {
	char *res = d;
	if ( !d || !o ) return res;

	while ( *d ) d++;
	while ( *o ) *d++ = *o++;
	*d = 0;

	return res;
}

char *strncat( char *d, char *o, size_t n ) {
	char *res = d;
	if ( !d || !o ) return res;

	while ( *d ) d++;
	while ( *o && n > 0 ) {
		*d++ = *o++;
		n--;
	}

	*d = 0;
	
	return res;
}
