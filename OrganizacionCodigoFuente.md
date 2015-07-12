# Directorios #
  * progs
    * include
      * prog1
      * prog2
      * ...
      * progN
    * src
      * prog1
      * prog2
      * ...
      * progN
  * kernel
    * include
      * proc
      * mm
      * int
      * fs
      * ...
    * src
      * proc
      * mm
      * int
      * fs
      * ...
  * bootloader
    * include
    * src

# Forma de Uso #
Al tenerlo separado de esta forma, si quiero acceder a estructuras o funciones definidas por el administrador de memoria, basta con incluir un archivo de la siguiente manera:

```
#include <mm/bitmap.h>
#include <mm/alloc_page.h>
...
```

Lo cual queda bonito :-)