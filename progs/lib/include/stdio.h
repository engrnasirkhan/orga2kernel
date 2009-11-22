#ifndef __STDIO__H__
#define __STDIO__H__

#include <config.h>
#include <stdarg.h>
#include <types.h>
CDECLSTART

#define EOF (-1)
#define NULL ((void *) 0)

typedef struct {
	int fileno;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

extern int printf( const char *fmt, ... );
extern int sprintf( char *buffer, const char *fmt, ... );
extern int snprintf( char *buffer, size_t size, const char *fmt, ... );

extern int vprintf( const char *fmt, va_list ap );
extern int vsprintf( char *buffer, const char *fmt, va_list ap );
extern int vsnprintf( char *buffer, size_t size, const char *fmt, va_list ap );

extern int putchar( int c );
extern int puts( const char *str );

extern size_t fread( void *ptr, size_t tam, size_t n, FILE *f );
extern size_t fwrite( void *ptr, size_t tam, size_t n, FILE *f );
extern void clearerr( FILE *f );
extern int feof( FILE *f );
extern int ferror( FILE *f );
extern int fileno( FILE *f );

CDECLEND
#endif
