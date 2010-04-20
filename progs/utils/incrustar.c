#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

int main( int argc, char **argv ) {
	FILE *in, *out;
	int c;
	int letras = 80;

	/* Abrimos los archivos */
	if ( argc == 3 ) {
		if ( !( in = fopen( argv[1], "rb" ) ) ) {
			perror( "fopen" );
			return -1;
		}
		if ( !( out = fopen( argv[2], "wt" ) ) ) {
			perror( "fopen" );
			fclose(in);
			return -1;
		}
	} else if ( argc == 2 ) {
		if ( !( in = fopen( argv[1], "rb" ) ) ) {
			perror( "fopen" );
			return -1;
		}
		out = stdout;
	} else {
		in = stdin;
		out = stdout;
	}

	if ( argc > 1 ) {
		for ( c = 0; argv[1][c]; c++ ) {
			if ( argv[1][c] == '.' ) argv[1][c] = '_';
		}
	}
	fprintf( out, "unsigned char %s[] __attribute__((aligned(4096))) = {", in == stdin ? "stdin" : basename(argv[1]) );
	while ( ( c = fgetc( in ) ) != EOF ) {
		if ( letras >= 80 ) { letras = 0; fputs( "\n\t", out ); }
		fprintf( out, "0x%02x, ", c );
		letras += 6;
	}
	fprintf( out, "\n};" );

	fclose(in);
	fclose(out);
	return 0;
}
