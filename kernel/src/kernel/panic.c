#include <asm/asm.h>

void debug( const char *fmt, ... ) {
	while ( *fmt )
		kputc( *fmt++ );
}

// TODO: Hacerlo bien.
void panic( const char *fmt, ... ) {
	debug(fmt);
	cli();
	for (;;) hlt();
}
