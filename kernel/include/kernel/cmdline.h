#ifndef __CMDLINE__H__
#define __CMDLINE__H__

#include <asm/asm.h>

void cmdline_init( const char *cmdline ) __init;
const char *cmdline_get_string( const char *name );
int cmdline_get_int( const char *name );

#endif
