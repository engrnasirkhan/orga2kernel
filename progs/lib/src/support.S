.sect .text
.global __umoddi3
.global __udivdi3

# unsigned long long __umoddi3 (unsigned long long a, unsigned long long b) { return a % b; }
# Esta función es emitida por el gcc cuando quiere "modulear" enteros sin signo
# de 64 bits. convertir_entero(...) la está usando.
#__umoddi3:
#	ret

# unsigned long long __udivdi3 (unsigned long long a, unsigned long long b) { return a / b; }
# Esta función es emitida por el gcc cuando quiere dividir enteros sin signo
# de 64 bits. convertir_entero(...) la está usando.
#__udivdi3:
#	ret
