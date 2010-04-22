#include <string.h>
#include <types.h>

#define ROWS 25
#define COLS 80
#define ESC 0x1b
static int _posx = 0, _posy = 0;
static short *_video = (short *) 0xB8000;
static int attr = 0x07;

static void _scrollup() {
	if (_posy < ROWS) return;

	memcpy( _video, _video + COLS, (ROWS-1) * COLS * 2 );
	memset( _video + (ROWS-1) * COLS, 0, COLS * 2 );
	_posy = ROWS - 1;
}

static void _putc( char c ) {
	switch ( c ) {
		/* Retorno de carro. */
		case '\n':
			_posy++;
			_posx = 0;
			_scrollup();
			break;

		case '\r':
			_posx = 0;
			break;

		case '\b':
			_posx--;
			if ( _posx < 0 ) {
				_posx = 0;
				_posy--;
				if ( _posy < 0 ) {
					_posy = 0;
				}
			}
			break;

		case '\t':
			_posx = (_posx + 8) & ~7;
			if ( _posx >= COLS ) {
				_posx = 0;
				_posy++;
				_scrollup();
			}
			break;

		default:
			_video[ _posy * COLS + _posx ] = 0x0700 | c;
			_posx++;
			if ( _posx >= COLS ) {
				_posx = 0;
				_posy++;
			}
			_scrollup();
	}
}

ssize_t write( int fd, const void *buf, size_t num ) {
	const char *datos = buf;

	// TODO: Llamada al sistema
	if ( fd != 1 ) return -1;

	while ( num-- )
		_putc( *datos++ );

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
