#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
	printf ( "Argumentos: %d\n", argc );
	for ( int i = 0; i < argc; i++ )
		printf ( "Argumento %d: %s\n", i+1, argv[i] );

	puts ( "Numeros aleatorios:" );
	srand( time( NULL ) );
	for ( int i = 0; i < 10; i++ ) {
		printf ( "%d ", rand() );
	}

	volatile int no_optimizar;
	for (;;) no_optimizar = 1;
}
