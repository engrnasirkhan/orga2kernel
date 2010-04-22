#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void main() {
	pid_t pid = getpid();
	int i;
	for ( i = 0; ; i++ )
		printf( "Proceso %d: %d.               \r", pid, i );
}

/*
int main( const char *argv ) {
	printf ( "Argumentos: %s\n", argv );
	puts ( "Numes aleatorios:" );
	for ( int i = 0; i < 10; i++ )
		printf ( "%d ", rand() );

	if ( !strcmp( argv, "/boot/ej1 Hola Mundo :-)" ) )
		return 123456789;
	return 987654321;
}
*/
/*
int main(int argc, char **argv) {
	printf ( "Argumentos: %d\n", argc );
	for ( int i = 0; i < argc; i++ )
		printf ( "Argumento %d: %s\n", i+1, argv[i] );

	puts ( "Numeros aleatorios:" );
	srand( time( NULL ) );
	for ( int i = 0; i < 10; i++ ) {
		printf ( "%d ", rand() );
	}

	return 0;
}*/
