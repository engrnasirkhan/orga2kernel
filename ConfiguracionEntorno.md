Primero que nada:

<img src='http://modmyi.com/forums/attachments/skinning-themes-discussion/1681d1190076512-request-hhgttg-dont-panic-dont-panic.png' width='100' />

# Paquetes a instalar #

  * nasm = compilador de assembler
  * gcc = compilador de c/c++

pueden instalarlos todos juntos asi

> `$ sudo apt-get install nasm gcc gdb`

# Bochs #

## Instalacion ##

  1. Bajar el source de [aca](http://sourceforge.net/projects/bochs/files/bochs/2.4.2/bochs-2.4.2.tar.gz/download)
  1. Descomprimirlo
  1. `$ ./configure --enable-disasm --enable-debugger --prefix=<bochs path>` ([mas opciones de coniguracion](http://bochs.sourceforge.net/doc/docbook/user/compiling.html#CONFIG-OPTS))
  1. `$ make`
  1. `$ make install`

## Configuración ##

### Bash command ###
Ahora ya estaria instalado en la carpeta "bochs path", solo falta configurar bash para que descubre donde esta bochs:
  1. Editar el archivo ".bashrc" en su carpeta home con su editor preferido.
  1. Agregar la siguiente linea al final del archivo: `export PATH+=":<bochs path>/bin/"`
  1. `$ source .bashrc`