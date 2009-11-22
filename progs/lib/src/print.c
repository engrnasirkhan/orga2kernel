#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <types.h>

#define BUFFER_SIZE 2048

// Requiere: size(buffer) >= suficiente.
// Requiere: buffer = último elemento del buffer.
static char *convertir_entero( char *buffer, unsigned long long numero, int signado, int rellenar, int precision, int anchura, int signo, int base, int mayusculas ) {
	char digitosMayusculas[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	char digitosMinusculas[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	// Guardamos el signo.
	if ( signado && (signed long long)numero < 0 ) {
		signo = '-';
		numero = -(signed long long)numero;
	} else if ( signo )
		signo = '+';

	*buffer-- = 0; // Fin del buffer.

	while ( numero ) {
		int remainder = numero % base;
		numero /= base;
		*buffer-- = mayusculas ? digitosMayusculas[remainder] : digitosMinusculas[remainder];
		anchura--;
	}

	if ( rellenar && signo ) {
		while ( anchura > 1 ) {
			*buffer-- = '0';
			anchura--;
		}
		*buffer-- = signo;
		anchura--;
	} else if ( rellenar ) {
		while ( anchura > 0 ) {
			*buffer-- = '0';
			anchura--;
		}
	} else if ( signo ) {
		*buffer-- = signo;
		anchura--;
	}

	while ( anchura > 0 ) {
		*buffer-- = ' ';
		anchura--;
	}

	return buffer + 1;
}

int vsnprintf( char *buffer, size_t n, const char *fmt, va_list ap ) {
	enum { IDLE, OPCION, ANCHURA, PRECISION, PRECISION_2, LONGITUD, CONVERSION, DONE, ERROR } estado = IDLE;
	char *end = buffer + n;
	int precision = -1;
	int anchura = -1;
	int signo = 0, rellenar = 0;	
	int x64 = 0;
	char *old_buffer = buffer;

	while ( *fmt && buffer != end ) {
		switch ( estado ) {
			case IDLE:
				if ( *fmt == '%' ) {
					fmt++;
					estado = OPCION;
				} else
					*buffer++ = *fmt++;
				break;

			case OPCION:
				if ( *fmt == '+' ) {
					signo = 1;
					fmt++;
				}
				else if ( *fmt == '0' ) {
					rellenar = 1;
					fmt++;
				}
				else
					estado = ANCHURA;
				break;

			case ANCHURA:
				if ( isdigit( *fmt ) ) {
					if ( anchura < 0 ) anchura = 0;
					anchura *= 10;
					anchura += *fmt - '0';
					fmt++;
				} else if ( *fmt == '.' ) {
					fmt++;
					estado = PRECISION;
				} else
					estado = LONGITUD;
				break;

			case PRECISION:
				if ( isdigit( *fmt ) ) {
					precision = *fmt - '0';
					fmt++;
					estado = PRECISION_2;
				} else
					estado = ERROR;
				break;

			case PRECISION_2:
				if ( isdigit( *fmt ) ) {
					precision *= 10;
					precision += *fmt - '0';
					fmt++;
				} else
					estado = LONGITUD;
				break;

			case LONGITUD:
				if ( *fmt == 'l' && *(fmt + 1) == 'l' ) {
					x64 = 1;
					fmt += 2;
				} else if ( *fmt == 'h' || *fmt == 'l' || *fmt == 'z' || *fmt == 't' ) {
					x64 = 0;
					fmt++;
				} else
					estado = CONVERSION;
				break;

			case CONVERSION:
				if ( *fmt == 'd' || *fmt == 'i' ) {
					// Decimal
					char buff[128];
					char *ptr;
					if (x64)
						ptr = convertir_entero( buff + 128, (unsigned long long) va_arg(ap, long long), 1, rellenar, precision, anchura, signo, 10, 0 );
					else
						ptr = convertir_entero( buff + 128, (unsigned long long) va_arg(ap, int), 1, rellenar, precision, anchura, signo, 10, 0 );
					while ( *ptr && buffer != end )
						*buffer++ = *ptr++;
					if ( buffer == end ) {
						*buffer = 0;
						return strlen(old_buffer);
					}
					estado = DONE;
				} else if ( *fmt == 'x' || *fmt == 'X' || *fmt == 'o' || *fmt == 'u' || *fmt == 'p' ) {
					int base = 10;
					int mayusculas = 0;
					char *ptr;
					char buff[128];

					if ( *fmt == 'x' || *fmt == 'X' || *fmt == 'p' ) base = 16;
					else if ( *fmt == 'o' ) base = 8;

					if ( *fmt == 'X' ) mayusculas = 1;

					if (x64)
						ptr = convertir_entero( buff + 128, va_arg(ap, unsigned long long), 0, rellenar, precision, anchura, signo, base, mayusculas );
					else
						ptr = convertir_entero( buff + 128, va_arg(ap, unsigned int), 0, rellenar, precision, anchura, signo, base, mayusculas );

					if ( *fmt == 'p' ) {
						if (buffer != end) *buffer++ = '0';
						if (buffer != end) *buffer++ = 'x';
					}

					while ( *ptr && buffer != end )
						*buffer++ = *ptr++;

					if ( buffer == end ) {
						*buffer = 0;
						return strlen(old_buffer);
					}
					estado = DONE;
				} else if ( *fmt == 'c' ) {
					while ( anchura > 1 && buffer != end ) {
						*buffer++ = rellenar ? '0' : ' ';
						anchura--;
					}
					if ( buffer != end ) *buffer++ = (char) va_arg(ap, int);
					if ( buffer == end ) {
						*buffer = 0;
						return strlen(old_buffer);
					}
					estado = DONE;
				} else if ( *fmt == 's' ) {
					const char *ptr = va_arg(ap, const char *);
					int ancho = strlen(ptr);
					if ( precision >= 0 )
						ancho = precision < ancho ? precision : ancho;

					while ( anchura > ancho && buffer != end ) {
						*buffer++ = rellenar ? '0' : ' ';
						anchura--;
					}

					if ( precision >= 0 ) {
						while ( *ptr && buffer != end && precision > 0 ) {
							*buffer++ = *ptr++;
							precision--;
						}
					} else {
						while ( *ptr && buffer != end )
							*buffer++ = *ptr++;
					}

					if (buffer == end) {
						*buffer = 0;
						return strlen(old_buffer);
					}
					estado = DONE;
				} else if ( *fmt == 'f' ) {
					// TODO: Float
				} else if ( *fmt == '%' ) {
					// % :-P
					*buffer++ = '%';
					estado = DONE;
				}
				break;

			case DONE:
			case ERROR:
				precision = -1;
				anchura = -1;
				signo = 0;
				rellenar = 0;
				fmt++; // Consumimos un caracter.
				estado = IDLE;
				break;
		}
	}

	*buffer = 0;
	return strlen(old_buffer);
}

int printf( const char *fmt, ... ) {
	va_list ap;
	int res;
	va_start(ap, fmt);
	res = vprintf( fmt, ap );
	return res;
}

int sprintf( char *buffer, const char *fmt, ... ) {
	va_list ap;
	int res;
	va_start(ap, fmt);
	res = vsprintf( buffer, fmt, ap );
	return res;
}

int snprintf( char *buffer, size_t size, const char *fmt, ... ) {
	va_list ap;
	int res;
	va_start(ap, fmt);
	res = vsnprintf( buffer, size, fmt, ap );
	return res;
}

int vprintf( const char *fmt, va_list ap ) {
	int res;
	char buffer[BUFFER_SIZE];
	res = vsnprintf( buffer, BUFFER_SIZE, fmt, ap );
	return fwrite( buffer, 1, res, stdout );
}

int vsprintf( char *buffer, const char *fmt, va_list ap ) {
	int res;
	res = vsnprintf( buffer, BUFFER_SIZE, fmt, ap );
	return res;
}
