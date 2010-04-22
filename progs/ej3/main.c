#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void main() {
	int caracteres = 0;
	char caracter = '=';
	for (;;) {
		putchar( caracter );
		caracteres++;
		if ( caracteres == 80 * 25 )	{
			caracteres = 0;
			caracter = caracter == '=' ? '+' : '=';
		}
	}
}
