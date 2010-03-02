#ifndef __KMALLOC__H__
#define __KMALLOC__H__

#include <asm/types.h>

//cantidad minima de espacio para llamar a morecore, si se pide menos, se pide NALLOC en su lugar
#define NALLOC  1024

typedef long align_t;

//Header del bloque de memoria libre
union header{
    struct {
        //puntero al proximo bloque libre
        union header *ptr;
        //Tamaño del bloque
        uint32_t size;       
    } s;
    
    //Solamente para alinear bien el header (el tipo align_t segun K&R puede variar segun la maquina)
    align_t x;
};

typedef union header header_t;

//Devuelve un puntero a block_size bytes listos para usar
//No controla que se use bien, por lo que cualquier mal uso rompe toda la estructura que controla el heap
ptr_t kmalloc(uint32_t block_size);

//Libera el bloque apuntado por ptr
void kfree(ptr_t ptr);

//Intenta conseguir mas espacio para el heap del kernel
//Si no consigue, devuelve NULL
header_t* morecore(uint32_t size);

//Agranda el heap el tamaño que sea necesario
uint8_t* sbrk(uint32_t size);
#endif
