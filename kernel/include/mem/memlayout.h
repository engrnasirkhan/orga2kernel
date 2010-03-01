#ifndef __MEMLAYOUT__H__
#define __MEMLAYOUT__H__

#define KERNEL_PHYSICAL_START   0x00100000
#define KERNEL_VIRTUAL_START    0x80100000

//Definimos el heap para el kernel
#define KERNEL_HEAP_START       0xC0000000

//transforms a given kernel virtual address into a physical address
#define KVA2PA(x)   x - 0x80000000
//inverse of KVA2PA
#define PA2KVA(x)   x + 0x80000000

#define PAGESIZE    0x1000

#define KERNEL_PAGESIZE   (0x400000)

#endif
