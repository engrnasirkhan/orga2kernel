
-la gdt primero la arme con c pero despues volvi a la de assembler
con las macros que defini me resulta mucho mas  clara 
miren las macros a ver que les parece.

-grub define una gdt y me deja en modo protegido y con a20 
pero para saber donde tengo la gdt y poner la mia 
cambie la gdt.

- Cuando se linkea el kernel 
idt.o necesita estar primero 
asi resuelve bien la ubicacion de  las 
rutinas isr[i] la idt
sino la idt queda con direcciones incorrectas
Fijense que en idt.asm esta definido ORIGIN que indica
donde esta ubicado el comienzo de la seccion texto
Si se encuentra una forma mejor de resolver esto bienvenida sea

- defini una rutina que se llama  "print" (que original que estube)
que corre todas las lineas de la pantalla hacia arriba una posicion
y en la ultima linea imprime el string pasado como parametro que 
debe terminar en 0. De esa menera detecta el final. No valida 
nada asi que si pasan un string mas largo va a reventar por que 
la pantalla la tengo en un segmento de la gdt que tiene la 
base y el limite justos para la pantalla.

- las rutinas isr que son las de la idt lo que hacen es
imprimir por pantalla isr[i] para indicar en que interrupcion o excepcion 
se entro. 

-para definir el stack cree un array que al linkear va a quedar en bss 
alineado a 4k fijense con objdump -h para ver como estan las secciones 
del kernel.elf hay algo mas interezante para esto en osdev y mejor 
echo pero bue. a otra cosa interezante es en el gdb una vez levantado 
el kernel hacer  "info address label" con los labels del kernel
para ver donde estan ubicados 

-en segments.mac tengo las macros para la gdt, tambien tendrian que estar
las de la idt

-para compilar "compilar.sh"

-la rutina print esta en print_protegido.asm 
hace uso del segmento 0x18 de la gdt para el video

-adjunto tambien el .gdbinit para que puedan correrlo rapido, 
lo tiran en el home, despues  corren bochs -q (el bochs compilado para gdt-stub) en 
esta carpeta y luego paraditos en la misma carpeta con otra terminal corren gdb. 
Ahi ponen continuan para que grub haga su trabajo y cuando le dan ok a grub, 
en la ventana del gdb van a quedar parados en el codigo fuente. 

si quieren en alguna linea ver el asm "layout asm" y para volvel "layout src"
para ver los registros en cualquiera de los layouts "layout regs" te los pone arriba.
