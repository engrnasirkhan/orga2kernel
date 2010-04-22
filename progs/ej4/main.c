#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <unistd.h>

void cuadrado( int x, int y, int ancho, int alto, int color ) {
	static unsigned short *video = (unsigned short *) 0xb8000;

	for ( int i = y; i < y+alto; i++ ) {
		for ( int j = x; j < x+ancho; j++ ) {
			video[ i * 80 + j ] = color << 8 | ' ';
		}
	}
}

void main() {
	//pid_t pid = getpid();
	int i;
	int color = 0;
	int colores[] = { 0x7f, 0x24, 0x64 };
	printf( "Proceso 4:"  );

	for (;;) {
		cuadrado( 10, 10, 60, 5, colores[color] );
		color = (color + 1) % (sizeof(colores)/sizeof(int));
		for ( i = 0; i < 15000; i++ ) {
			__asm__ __volatile__ ( "nop" );
		}
	}
}
