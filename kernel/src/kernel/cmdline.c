#include <asm/asm.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <kernel/cmdline.h>

char g_cmdline[256];


/* Esta función toma una cadena de palabras y
 * reemplaza las secuencias de espacios >= 1 por un \0
 */
void cmdline_init( const char *cmdline ) {
	int i = 0;
	int primer_espacio = 1;
	char c;

	if ( !cmdline ) return;

	// Primero salteamos cualquier espacio que pueda haber.
	while ( isspace(*cmdline) ) cmdline++;

	// Ahora procesamos.
	while ( (c = *cmdline) && i < sizeof(g_cmdline) ) {
		if ( isspace(c) ) {
			if ( primer_espacio ) {
				primer_espacio = 0;
				g_cmdline[i++] = 0;
			}
		} else {
			primer_espacio = 1;
			g_cmdline[i++] = c;
		}
		cmdline++;
	}

	// Rellenamos lo que queda con ceros.
	if ( i == sizeof(g_cmdline) ) i--; // Nos aseguramos que termine en \0
	for ( ; i < sizeof(g_cmdline); i++ ) g_cmdline[i] = 0;
}

const char *cmdline_get_string( const char *variable ) {
	// Buscamos una cadena del estilo variable=valor o bien valor a secas
	int d = 0, o = 0;

	while ( d < sizeof(g_cmdline) ) {
		while ( g_cmdline[d] == variable[o] && variable[o] ) {
			d++;
			o++;
		}

		if ( g_cmdline[d] == '=' && variable[o] == 0 )
			return g_cmdline + d + 1;
	
		if ( g_cmdline[d] == 0 && variable[o] == 0 )
			return "true";

		o = 0;
		while ( g_cmdline[d] ) d++;
		d++;
	}

	return (const char *) 0;
}

int cmdline_get_int( const char *variable ) {
	const char *res = cmdline_get_string( variable );
	if ( !strcmp( res, "true" ) ) return 1;
	return atoi( res ); // NULL = 0
}
