#ifndef __MEMLAYOUT__H__
#define __MEMLAYOUT__H__

#define KERNEL_PHYSICAL_START       0x00100000
#define KERNEL_VIRTUAL_START        0x80100000

//Definimos el heap para el kernel
#define KERNEL_HEAP_START           0xC0000000

//Definimos un espacio de direcciones virtuales para mapear punteros a las tablas de paginacion
#define KERNEL_PAGING_TABLES_VA     0xF0000000

//transforma una direccion virtual del kernel en una direccion fisica
#define KVA2PA(x)   (x - 0x80000000)
//inversa de KVA2PA
#define PA2KVA(x)   (x + 0x80000000)
//transforma una direccion fisica correspondiente a una tabla de pagina a su direccion virtual
#define PTPA2KVA(x) ((x/PAGESIZE) + KERNEL_PAGING_TABLES_VA)
//opuesto a ptpa2kva
#define KVA2PTPA(x) ((x/PAGESIZE) - KERNEL_PAGING_TABLES_VA)


#define PAGESIZE    0x1000

#define KERNEL_PAGESIZE   (0x400000)

#endif
