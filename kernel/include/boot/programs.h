#ifndef __PROGRAMS__H__
#define __PROGRAMS__H__

/** 
 * Cada módulo que carga el grub se compone en primer lugar de esta estructura.
 * La función del punto de entrada debe tener la siguiente firma:
 * void punto_de_entrada( void *esp, const char *cmdline );
 * donde esp = ESP y cmdline = la dirección del cmdline del módulo.
 *
 * Asumimos que el archivo está estructurado de esta manera:
 * [ tabla | sección de código | sección de datos | seccioń bss ]
 * De modo que el inicio de la sección de datos es el fin de la
 * sección de código, etc.
 */
typedef struct {
	char magic[4]; // "EXE\0"
	void *va_entry; // Punto de entrada del programa (dirección virtual).
	void *va_text; // Dirección de comienzo de la sección de código.
	void *va_data; // Dirección de comienzo de la sección de datos.
	void *va_bss;   // Dirección de comienzo de la sección bss (dirección virtual).
	void *va_bssend; // Dirección de fin de la sección bss (dirección virtual).
} programs_t;


#endif // __PROGRAMS__H__
