#ifndef __VMM__H__
#define __VMM__H__

#include <asm/types.h>

//Cantidad minima de espacio para llamar a morecore, si se pide menos, se pide NALLOC en su lugar
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

/**
 * Devuelve un puntero a block_size bytes listos para usar dentro del heap del kernel
 * No realiza verificacion de limites, por lo que escribir mas alla del tamaño pedido puede ocasionar sobreescribir estructuras de control
 *
 * @param block_size Tamaño del nuevo bloque pedido
 * @return ptr_t Puntero al bloque de memoria solicitado
 * @see kfree
 */
ptr_t kmalloc(uint32_t block_size);

/**
 * Libera un bloque de memoria solicitado con kmalloc.
 *
 * @param prt Puntero al bloque de memoria conseguido con kmalloc
 * @see kmalloc
 */
void kfree(ptr_t ptr);

#endif //__VMM__H__
