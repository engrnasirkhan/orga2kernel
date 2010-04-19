#ifndef __MEMLAYOUT__H__
#define __MEMLAYOUT__H__

#define KERNEL_PHYSICAL_START       0x00100000
#define KERNEL_VIRTUAL_START        0x80100000
#define KERNEL_MEMMAP               0x80000000

//Definimos el heap para el kernel
#define KERNEL_HEAP_START           0xC0000000

//Definimos un espacio de direcciones virtuales para mapear punteros a las tablas de paginacion
#define KERNEL_PAGING_TABLES_VA     0xF0000000

//transforma una direccion virtual del kernel en una direccion fisica
#define KVA2PA(x)   (((uint32_t) (x)) - 0x80000000)
//inversa de KVA2PA
#define PA2KVA(x)   (((uint32_t) (x)) + 0x80000000)

#define PAGE_SIZE          (0x1000)
#define LARGE_PAGE_SIZE   (0x400000)

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_CS 0x18
#define USER_DS 0x20

#endif
