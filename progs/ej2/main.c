#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int contador;

void main() {
	int pid = getpid();
	for ( ; contador < 20000; contador++ ) {
		printf ( "Proceso %d: %d\n", pid, contador );
	}
	exit( 0 );
	return 0;
}
