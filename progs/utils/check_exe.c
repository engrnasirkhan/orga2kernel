#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#define COLOR_INFORMATION "\033[1;32m"
#define COLOR_ERROR       "\033[1;31m"
#define COLOR_WARNING     "\033[1;33m"
#define COLOR_NORMAL      "\033[m"

/* La redefinimos acá para que el void* no se confunda con un puntero de 64 bits en amd64 */
typedef struct {
	char magic[4]; // "EXE\0"
	unsigned int va_entry; // Punto de entrada del programa (dirección virtual).
	unsigned int va_text; // Dirección de comienzo de la sección de código.
	unsigned int va_data; // Dirección de comienzo de la sección de datos.
	unsigned int va_bss;   // Dirección de comienzo de la sección bss (dirección virtual).
	unsigned int va_bssend; // Dirección de fin de la sección bss (dirección virtual).
} programs_t;

static struct option longoptions[] = {
	{ "verbose", no_argument, NULL, 'v'  },
	{ "help", no_argument, NULL, 'h'  },
	{ NULL, 0, NULL, 0 }
};

static void uso( int salir ) {
	fprintf ( stderr, "Uso: check_exe [-vh] archivo_1 [archivo2 archivo3 ... archivoN]\n" );
	fprintf ( stderr, " * -v: Verbose.\n" );
	fprintf ( stderr, " * -h: Help.\n" );
	if (salir) exit( salir );
}

int main( int argc, char **argv ) {
	int verbose, c;
	const char *colores[4] = {
		COLOR_NORMAL,
		COLOR_INFORMATION,
		COLOR_WARNING,
		COLOR_ERROR
	};
	const char *colores_stderr[4] = {
		COLOR_NORMAL,
		COLOR_INFORMATION,
		COLOR_WARNING,
		COLOR_ERROR
	};
	enum {
		NORMAL = 0,
		INFORMATION = 1,
		WARNING = 2,
		ERROR = 3
	};

	verbose = 0;
	opterr = 0;
	if (!isatty(1)) {
		for ( int i = 0; i < 4; i++ )
			colores[i] = "";
	}

	if (!isatty(2)) {
		for ( int i = 0; i < 4; i++ )
			colores_stderr[i] = "";
	}

	while ( (c = getopt_long( argc, argv, "hv", longoptions, NULL )) != -1 ) {
		switch (c) {
			case 'h':
				uso(1);
				break;

			case 'v':
				verbose = 1;
				break;

			case '?':
			default:
				fprintf ( stderr, "Opción no reconocida: %c\n", optopt );
				uso(-1);
				break;
		}
	}

	if ( optind == argc ) {
		fprintf ( stderr, "Falta archivo.\n" );
		uso(-1);
	}

	for ( int i = optind; i < argc; i++ ) {
		FILE *f = fopen( argv[i], "rb" );
		programs_t prog;

		if ( !f ) {
			fprintf ( stderr, "%sWarning: No se pudo abrir %s%s\n", colores_stderr[WARNING], argv[i], colores_stderr[NORMAL] );
			continue;
		}

		if ( fread( &prog, 4, 6, f ) < 6 || *((unsigned int *) prog.magic) != 0x00455845  ) {
			printf ( "%sError: %s no es un archivo ejecutable.\n%s", colores[ERROR], argv[i], colores[NORMAL] );
			fclose ( f );
			continue;
		}


		if (verbose) {
			printf( "%s"
				"%s:\n"
				"* Tipo: %s\n"
				"* Entry point: 0x%08x\n"
				"* Text: 0x%08x\n"
				"* Data: 0x%08x\n"
				"* BSS: 0x%08x\n"
				"* End: 0x%08x\n"
				"%s",
				colores[INFORMATION],
				argv[i],
				prog.magic,
				prog.va_entry,
				prog.va_text,
				prog.va_data,
				prog.va_bss,
				prog.va_bssend,
				colores[NORMAL]
			);
		} else {
			printf ( "%s%s es un archivo ejecutable.\n%s",
				colores[INFORMATION],
				argv[i],
				colores[NORMAL]
			);
		}
		fclose ( f );	
	}
	return 0;
}
