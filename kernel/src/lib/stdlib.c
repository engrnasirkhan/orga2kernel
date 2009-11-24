#include <ctype.h>

int atoi( const char *str ) {
	int res = 0;
	int base = 10;

	if ( !str ) return 0;

	// Elegimos la base :-)
	if ( *str == '0' ) {
		base = 8;
		str++;
		if ( *str == 'x' ) {
			base = 16;
			str++;
		}
	}

	// Convertimos
	while ( *str ) {
		if ( base == 16 && !isxdigit(*str) ) break;
		if ( base <= 10 && !isdigit(*str) ) break;

		res *= base;
		if ( isdigit(*str) ) res += *str - '0';
		else res += toupper(*str) - 'A' + 10;

		str++;
	}
	return res;
}
