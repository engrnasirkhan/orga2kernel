#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int contador;

void main() {
	pid_t pid = getpid();
	for ( ; contador < 20000; contador++ ) {
		printf ( "Proceso %d: %d\n", pid, contador );
	}
	exit( 0 );
}
