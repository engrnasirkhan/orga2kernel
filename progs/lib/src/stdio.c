#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <types.h>

static FILE __stdin = { 0 };
static FILE __stdout = { 1 };
static FILE __stderr = { 1 }; // alias a stdout.

FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *stderr = &__stderr;

int putchar( int c ) {
	if ( write( 1, &c, 1 ) != 1 )
		return EOF;
	return c;
}

int puts( const char *str ) {
	int resA, resB;
	char newline = '\n';

	resA = write( 1, str, strlen(str) );
	resB = write( 1, &newline, 1 );

	if ( resA < 0 || resB != 1 ) return EOF;
	return resA + 1;
}

size_t fread( void *ptr, size_t tam, size_t n, FILE *f ) {
	return EOF;
}

size_t fwrite( void *ptr, size_t tam, size_t n, FILE *f ) {
	return write( f->fileno, ptr, tam * n ) / tam;
}

void clearerr( FILE *f ) { }
int feof( FILE *f ) { return 0; }
int ferror( FILE *f ) { return 0; }
int fileno( FILE *f ) { if (f) return f->fileno; return -1; }
