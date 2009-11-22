/*struct unsigned_large {
	unsigned int low;
	unsigned int high;
};

static int bit( struct unsigned_large *num, int bit ) {
	if ( bit <= 31 ) return num->low & (2<<bit) ? 1 : -1;
	return num->high & (2<<(bit-32)) ? 1 : -1;
}

static void set_bit( struct unsigned_large *num, int bit, int e ) {
	if ( bit <= 31 ) {
		if ( e ) num->low |= (2<<bit);
		else num->low &= ~(2<<bit);
	} else {
		if ( e ) num->high |= (2<<(bit-32));
		else num->high &= ~(2<<(bit-32));
	}
}*/

/** TODO: Esto debería ser una división y módulo de 64 bits de verdad **/
unsigned long long __umoddi3 (unsigned long long int a, unsigned long long int b) {
	unsigned int res = (unsigned int) a % (unsigned int) b;
	return (unsigned long long) res;
}

unsigned long long __udivdi3 (unsigned long long int a, unsigned long long int b) {
	unsigned int res = (unsigned int) a / (unsigned int) b;
	return (unsigned long long) res;
}
