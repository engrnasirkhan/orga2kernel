set disassembly-flavor intel


#me conecto al debugger remoto del bochs
#que me espera para comenzar
target remote :1234
set confirm off
#agrego el archivo de simbolos para poder ver el codigo bien
add-symbol-file kernel.elf 0x100000

#breakpoint en el kmain del grub
b kmain

#veo la pantalla dividida arriba el fuente en c
#abajo los comandos 
layout src


#defino funciones para ver las estructuras 
#son medias chotas hay que ver mejor que se puede 
#hacer con este lenguaje del gdb 
define printgdt
x/4xg BEGIN_GDT
end

define printidt
x/32xg BEGIN_IDT
end
