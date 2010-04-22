#ifndef __PANIC__H__
#define __PANIC__H__

#include <asm/types.h>

void panic( const char *fmt, ... );
void panic_regs( struct registers *r, const char *fmt, ... );
void debug( const char *fmt, ... );
void dumpregs();
void dumpregs_regs( struct registers *r );
void stacktrace( int saltar, int cuantas );

#endif
