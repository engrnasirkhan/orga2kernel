#include <types.h>

ssize_t write( int fd, const void *buf, size_t num ) {
	static short *video = (short *) 0xB8000;
	const char *datos = buf;

	// TODO: Llamada al sistema
	if ( fd == 1 ) {
		while ( num-- ) {
			*video++ = (short) *datos++ | 0x7f00;
			if ( (int)video >= (0xB8000 + 80 * 25) )
				video = (short *) 0XB8000;
		}
	}

	return 0;
}

ssize_t read( int fd, const void *buf, size_t num ) {
	// TODO: Llamada al sistema
	return 0;
}

int close( int fd ) {
	// TODO: Llamada al sistema
	return 0;
}

int open( const char *camino, int flags, ... ) {
	// TODO: Llamada al sistema
	return 0;
}

pid_t fork( void ) {
	// TODO: Llamada al sistema.
	return 0;
}

pid_t wait( int *status ) {
	// TODO: Llamada al sistema.
	return 0;
}

pid_t waitpid( pid_t pid, int *status, int options ) {
	// TODO: Llamada al sistema.
	return 0;
}
