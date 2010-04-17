#include <asm/asm.h>
#include <mem/mmu.h>
#include <mem/vmm.h>

/**
 * Intenta agrandar el heap del kernel reservando mas espacio usand sbrk.
 * 
 * @param size Tamaño a agrandar
 * @return header_t* Devuelve un puntero al bloque de memoria conseguido, o NULL si no pudo agrandar el heap.
 * @see 
 */
static header_t* vmm_morecore(uint32_t size);

//Inicio de la lista de memoria libre
static header_t *base = (header_t*)KERNEL_HEAP_START; 

//Lista de bloques libres
static header_t *freep = NULL;

//Apunta al final del heap del kernel (inicia en 4k mas adelante porque incializamos el heap en 4k)
static uint8_t* kernel_heap_end = (uint8_t*)(KERNEL_HEAP_START + PAGE_SIZE);

ptr_t kmalloc(uint32_t nbytes)
{
    header_t *p, *prevp;
    uint32_t nunits;
    
    nunits = (nbytes + sizeof(header_t) -1)/sizeof(header_t) + 1;
    //nunits = nbytes + sizeof(header_t) - nbytes%sizeof(header_t);

    if((prevp = freep) == NULL)
    {
        //No hay una lista construida todavia
        base->s.ptr = freep = prevp = base;
        base->s.size = 0;
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

header_t* vmm_morecore(uint32_t size)
{
    uint8_t *cp;
    header_t *up;
    
    //como minimo agrandamos el heap en el tamaño NALLOC
    if(size < NALLOC)
    {
        size = NALLOC; 
    }
    
    //Veamos si el espacio que quieren entra entre kernel_heap_end y el comienzo de una nueva pagina
    //r es lo que le falta a kernel_heap_end para llegar a una direccion alineada a PAGESIZE
    uint32_t r = (-((uint32_t)kernel_heap_end))%PAGE_SIZE; 

    if(r < size)
    {
        //si piden algo mas grande que el espacio entre kernel_heap_end y la proxima pagina, entonces pedimos mas paginas !
        uint32_t frames_needed = size/PAGE_SIZE;
        if(r)
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
            uint32_t i;
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
    kernel_heap_end += size;
    cp = old_kernel_heap_end;
    
    up = (header_t*) cp;
    up->s.size = size;

    kfree((ptr_t)(up+1));
    
    return freep;
}

/*
uint8_t* sbrk(uint32_t size)
{
    //veamos si el espacio que quieren entra entre kernel_heap_end y el comienzo de una nueva pagina
    //r es lo que le falta a kernel_heap_end para llegar a una direccion alineada a PAGESIZE
    uint32_t r = (-((uint32_t)kernel_heap_end))%PAGE_SIZE; 

    if(r < size)
    {
        //si piden algo mas grande que el espacio entre kernel_heap_end y la proxima pagina, entonces pedimos mas paginas !
        uint32_t frames_needed = size/PAGE_SIZE;
        if(r)
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
            uint32_t i;
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
    kernel_heap_end += size;
    
    return old_kernel_heap_end;
}
*/
