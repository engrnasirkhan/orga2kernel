#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int contador;

int main( const char *argv ) {
	for (;;) {
		printf ( "Proceso 2: %d\n", contador++ );
	}
	return 0;
}
