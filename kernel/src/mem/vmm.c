#include <asm/asm.h>
#include <mem/mmu.h>
#include <mem/vmm.h>

/**
 * Intenta agrandar el heap del kernel reservando mas espacio usand vmm_resize_heap.
 * 
 * @param size Tamaño a agrandar
 * @return header_t* Devuelve un puntero al bloque de memoria conseguido, o NULL si no pudo agrandar el heap.
 * @see 
 */
static header_t* vmm_morecore(uint32_t size);

/**
 * Agranda el heap en nbytes bytes.
 *
 * @param nbytes Cantidad de bytes para agrandar el heap
 * @return uint8_t* Puntero al bloque de nbytes bytes.
 * @see vmm_morecore
 */
static uint8_t* vmm_resize_heap(uint32_t nbytes);

//Inicio de la lista de memoria libre
static header_t base;

//Lista de bloques libres
static header_t *freep = NULL;

//Apunta al final del heap del kernel (inicia en 4k mas adelante porque incializamos el heap en 4k)
static uint8_t* kernel_heap_end = (uint8_t*)(KERNEL_HEAP_START);

ptr_t kmalloc(uint32_t nbytes)
{
    header_t *p, *prevp;
    uint32_t nunits;
    
    nunits = (nbytes + sizeof(header_t) -1)/sizeof(header_t) + 1;

    if((prevp = freep) == NULL)
    {
        //No hay una lista construida todavia
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }
    for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr)
    {
        if(p->s.size >= nunits)
        {
            //hay espacio suficiente!
            if(p->s.size == nunits)
            {
                //justo el tamaño es exacto :)
                prevp->s.ptr = p->s.ptr;
            }
            else
            {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (ptr_t)(p+1);
        }
        if(p == freep)
        {
            if((p = vmm_morecore(nunits)) == NULL)
            {
                return NULL;
            }
        }
    }   
}

void kfree(ptr_t ap)
{
    header_t *bp, *p;
    bp = (header_t*)ap-1;

    for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    {
        if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
        {
            break;
        }
    }
    
    if(bp + bp->s.size == p->s.ptr)
    {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr  = p->s.ptr->s.ptr;
    }
    else
    {
        bp->s.ptr = p->s.ptr;
    }     
    
    if(p + p->s.size == bp)
    {
        p->s.size += bp->s.size;
        p->s.ptr  = bp->s.ptr;
    }
    else
    {
        p->s.ptr = bp;
    }
    
    freep = p;
}

header_t* vmm_morecore(uint32_t nunits)
{   
    uint8_t *cp;
    header_t *up;

    if(nunits < NALLOC)
    {  
        //Cantidad de unidades minima para agrandar el heap
        nunits = NALLOC;
    }
    
    cp = vmm_resize_heap(nunits * sizeof(header_t));
    if(cp == NULL)
    {
        return NULL;
    }  
    
    up = (header_t*) cp;
    up->s.size = nunits;
    kfree((ptr_t)(up+1));
    
    return freep;
}


static uint8_t* vmm_resize_heap(uint32_t nbytes)
{
    //Veamos si los nbytes entran entre el kernel_heap_end y el final de la pagina
    //r es lo que le falta a kernel_heap_end para llegar a una direccion alineada a PAGE_SIZE
    uint32_t r = (-((uint32_t)kernel_heap_end))%PAGE_SIZE; 

    if(r < nbytes)
    {
        //Si piden algo mas grande que el espacio entre kernel_heap_end y la proxima pagina, entonces pedimos mas paginas !
        //La cantidad de bytes que hay que reservar es
        uint32_t bytes_needed = nbytes - r;
        uint32_t frames_needed = bytes_needed/PAGE_SIZE;
        if(bytes_needed%PAGE_SIZE)
        {
            frames_needed++;
        }
        if(mmu_get_free_frame_count() < frames_needed){
            //La cantidad de lugar no alcanza, asi que por ahora no hacemos otra cosa mas que devolver NULL
            //Se podria intentar hacer lugar swapeando paginas, pero por ahora no sabemos hacer eso
            return NULL;
        }
        else
        {
            uint32_t va = (uint32_t)kernel_heap_end + r;
            while(frames_needed)
            {
                mmu_alloc_at_VA((pde_t*)PA2KVA(getCR3()), va, PAGE_SUPERVISOR|PAGE_RW, 1);
                frames_needed--;
                va += PAGE_SIZE;
            }
        }
    }
    uint8_t *old_kernel_heap_end = kernel_heap_end;
    //Alagargamos el heap
    kernel_heap_end = (uint8_t*) (nbytes + (uint32_t)kernel_heap_end);
    
    return old_kernel_heap_end;
}

uint32_t vmm_get_kernel_heap_end()
{
    return (uint32_t)kernel_heap_end;
}

