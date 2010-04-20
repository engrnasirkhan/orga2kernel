# Este es el primer archivo de _TODOS_ los programas
# Primero se encuentra esta tabla:
.sect .text
.global __start
.extern main

.long 0x00455845 # "EXE\0"
.long __start # Entry point :-)
.long __text_start
.long __data_start
.long __bss_start
.long __bss_end

### __start debería ser una función que tome
### * argumentos
### * variables de entorno
### y construya los datos necesarios para llamar a main( int argc, char **argv, char **envp );
### y luego llamar a exit() si main retorna...
### Más a futuro: Llamar a constructores y destructores de C++.
#__start:
#	pushl $0 # argv
#	pushl $0 # argc
#	call main
#	# Una excepción si llegamos acá.
#1:
#	cli
#	hlt
#	jmp 1b

__start:
	#pushl 4(%esp)
	call main
	#addl $4, %esp
	ret
