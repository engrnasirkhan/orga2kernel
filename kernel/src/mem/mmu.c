#include <screen/screen.h>
#include <asm/asm.h>
#include <asm/gdt.h>
#include <mem/mmu.h>
#include <mem/vmm.h>
#include <kernel/globals.h>
#include <kernel/panic.h>
#include <lib/string.h>

/**
 * Inicializa la PDT del kernel, mapeando la memoria fisica en forma lineal a partir
 * de KERNEL_VIRTUAL_START utilizando paginas de KERNEL_PAGE_SIZE (4mb). 
 *
 * @param kpdt Puntero a la page directory table del kernel
 * @see mmu_init
 * @see mmu_install_gdt
 */
static void mmu_init_paging(uint32_t *kpdt);

/**
 * Instala la GDT definitiva, quitando la dummy gdt utilizada durante el proceso de booteo.
 * @see mmu_init
 * @see mmu_init_paging
 */
static void mmu_install_gdt();

/**
 * Toma un puntero a un frame y lo almacena en la pila de frames libres.
 * Solo se debe hacer push de frames con cantidad de referencias igual a 0.
 *
 * @param page_frame Puntero al page_frame_t a liberar
 * @see mmu_pop_free_frame
 */
static void mmu_push_free_frame(page_frame_t* page_frame);

/**
 * Saca un frame del stack de frames libres y devuelve un puntero a este
 * 
 * @return Devuelve un puntero del tipo page_frame_t*. Si no hay frames libres, devuelve NULL
 * @see mmu_push_free_frame
 */
static page_frame_t* mmu_pop_free_frame();

/**
 * Devuelve la direccion fisica del frame pasado como parametro. Para ello se fija en el 
 * puntero al frame, y lo compara con el punter o al principio de la lista para conocer su posicion
 * y luego multiplicar convenientemente para conocer la direccion fisica del mismo
 *
 * @param page_frame Puntero al page_frame cuya physical address se quiere conocer
 * @return uint32_t
 * @see mmu_PA_2_page_frame
 */
static uint32_t mmu_page_frame_2_PA(page_frame_t* page_frame);

/**
 * Toma una direccion fisica y devuelve un puntero page_frame_t al frame correspondiente en el array
 * mem_page_frames. No quita el frame de la lista de libres.
 *
 * @param physical_address Direccion fisica del frame. Debe ser multiplo de PAGE_SIZE
 * @return Devuelve un puntero page_frame_t* al frame asociado, o NULL si la direccion fisica es invalida
 * @see mmu_page_frame_2_PA
 */
static page_frame_t* mmu_PA_2_page_frame(uint32_t physical_address);

/**
 * Dada una direccion fisica, devuelve el frame asociado a esta, pero lo quita de la lista de libres (si es que lo estaba)  
 * incrementando la cantidad de referencias.
 *
 * @param physical_address Direccion fisica del frame que se quiere obtener
 * @return Devuelve un puntero page_frame_t* al frame correspondiente a la direccion fisica, o NULL si la direccion fisica era invalida
 * @see  
 */
static page_frame_t* mmu_get_page_frame(uint32_t physical_address);

/**
 * Decrementa las referencias del frame pasado como parametro. Si llegan a ser nulas, se agrega a la pila de libres.
 * 
 * @param frame Puntero al frame a liberar
 * @see mmu_get_page_frame
 * @see mmu_push_free_frame
 */
static void mmu_free_page_frame(page_frame_t *frame);

/**
 * Asocia la direccion virtual va con el frame apuntado por page_frame para la PDT pasada como parametro, utilizando los permisos perm.
 * Si la va ya estaba siendo utilizada para otro mapeo, force_dealloc indica si se deshace el mapeo anterior o no.
 *
 * @param pdt Puntero a la PDT sobre la cual se va a realizar el mapeo.
 * @param page_frame Puntero a un marco de pagina
 * @param va Virtual address a utilizar para el mapeo
 * @param perm Permisos con los que se va a mapear la pagina
 * @param force_dealloc Si vale 1, fuerza el desmapeo de cualquier mapeo anterior.
 * @return uint8_t Devuelve E_MMU_SUCCESS si todo salio correctamente. E_MMU_INVALID_VA si la va ya estaba mapeada y force_dealloc==0. E_MMU_NO_MEMORY si no hubo memoria.
 * @see mmu_dirwalk
 */ 
static uint8_t mmu_alloc_page(pde_t *pdt, page_frame_t *page_frame, uint32_t va, uint8_t perm, uint8_t force_dealloc);

/**
 * Recorre las tablas y hace apuntar pte a la page table entry que corresponde de acuerdo a la va pasada como parametro y la PDT.
 *
 * @param pdt Puntero a la PDT sobre la cual se quiere realziar el walk.
 * @param va Virtual address
 * @param pte Puntero doble a una pte_t. Dentro de pte se guarda la pte correspondiente a la va para la PDT.
 * @param create_page_table Si la va no tiene una page table creada, create_page_table==1 hace que se cree dicha tabla, de lo contrario no se crea y mmu_dirwalk devuelve error
 * @return uint8_t Devuelve E_MMU_SUCCESS si todo salio correctamente. E_MMU_PTABLE_NOT_PRESENT si la tabla de paginas para la va no existe y create_page_table==0.
 *                 E_MMU_NO_MEMORY si no hubo memoria suficiente para realizar la operacion.                     
 */
static uint8_t mmu_dirwalk(pde_t *pdt, uint32_t va, pte_t **pte, uint8_t create_page_table, uint8_t ptable_domain);

/**
 * Devuelve un puntero a la la pte asociada a la va pasada como parámetro.
 *
 * @params pdt Puntero a la pdt sobre la cual buscar la pte
 * @params va Virtual Addess cuya pte asociada dentro de ptd se quiere buscar
 * @return pte_t* Devuelve un puntero a la pte asociada o NULL si no existe la page table
 * @see mmu_install_task_pdt
 * @see mmu_uninstall_task_pdt
 */
static pte_t* mmu_get_pte(pde_t *ptd, uint32_t va);

//Puntero al final del codigo del kernel (provisto por el linker)
extern uint8_t __kernel_end[];

static uint32_t kernel_physical_end = KVA2PA((uint32_t)__kernel_end);

//Total de memoria fisica disponible para administrar por el SO (es menor que la total)
static uint32_t available_physical_memory = 0;

//mem_frames es un arreglo de page_frame_t. Basicamente, permite encontrar en O(1) si un frame tiene referencias externas
static page_frame_t* mem_page_frames = NULL;

//stack de frames libres
static page_frame_t* free_page_frame_stack = NULL;

//Cantidad de frames en la memoria fisica disponible
static uint32_t page_frames_count = 0;

//cantidad de frames libres en la memoria fisica
static uint32_t free_page_frame_count = 0;

//la Page Directory Table del kernel
static pde_t kernel_pdt[1024]  __attribute__ ((aligned (4096)));

static void mmu_save_modules( multiboot_info_t *mbd, uint32_t *fa, uint32_t *la ) {
	memory_map_t *mmap;
	module_t *mod;
	uint32_t i;
	uint32_t last_address, last_address_backup;
	uint32_t paginas;

	if ( !(mbd->flags & 0x28) ) return;

	/* Obtengo la última dirección utilizable, que por ahora
	 * supongo que alcanza para copiar los módulos :-P
	 */
	for ( mmap = (memory_map_t *) PA2KVA(mbd->mmap_addr);
		(uint32_t) mmap < PA2KVA(mbd->mmap_addr + mbd->mmap_length);
		mmap = (memory_map_t *) ((uint32_t)mmap + mmap->size + sizeof(mmap->size)))
	{
		if ( mmap->type != 1 ) continue;
		last_address = mmap->base_addr_low + mmap->length_low;
	}

	/* La dejamos en un múltiplo de página. */
	last_address &= 0xFFFFF000;
	last_address = PA2KVA(last_address);
	last_address_backup = last_address;

	/* Copiamos los módulos. */
	kprint( "Copiando módulos: %d\n", mbd->mods_count );
	for ( i = 0, mod = (module_t *) PA2KVA(mbd->mods_addr);
		i < mbd->mods_count;
		i++, mod++ ) {
		paginas = (mod->mod_end - mod->mod_start) >> 12;
		if (paginas == 0) paginas++;
		last_address -= PAGE_SIZE * paginas;

		kprint( "memcpy(0x%x,0x%x,%d)\n", last_address, PA2KVA(mod->mod_start), mod->mod_end - mod->mod_start );

		memcpy( (void *)last_address, (void *) PA2KVA(mod->mod_start), mod->mod_end - mod->mod_start );	
		mod->mod_start = last_address;
		mod->mod_end = last_address + paginas * PAGE_SIZE;
	}

	*fa = last_address;
	*la = last_address_backup;
}

//Inicializa las estructuras relacionadas con la admin de memoria
void mmu_init(multiboot_info_t* mbd)
{  
	uint32_t fa, la, i;
    if(mbd->flags & 0x0001)
    { 
        available_physical_memory = mbd->mem_lower + mbd->mem_upper;
    }
    
    //El array de page_frame_t empieza justo al final del codigo del kernel
    mem_page_frames = (page_frame_t*)(PA2KVA(kernel_physical_end));
    
    //inicializamos la cantidad de paginas de LARGE_PAGE_SIZE que ocupa el kernel
    uint32_t kernel_pages_count = 0;

	mmu_save_modules( mbd, &fa, &la );
    
    //Si tenemos disponible el mapa de memoria del GRUB, usarlo!
    if(mbd->flags & 0x0020)
    {      
        memory_map_t *mmap;

        kprint("Verificando RAM...");
        for(mmap = (memory_map_t *) (mbd->mmap_addr + 0x80000000);
            (unsigned long) mmap < (mbd->mmap_addr + mbd->mmap_length + 0x80000000);
            mmap = (memory_map_t *) ((unsigned long) mmap
                                     + mmap->size + sizeof (mmap->size)))
        {
            //Solo vamos a administrar las partes reservadas para el SO
            //El resto esta reservado para el bios y otras cosas.
            if(mmap->type == 0x01)
            {
                //Inicializamos la lista de frames libres y el arreglo de referencias
                uint32_t i;
                for(i = mmap->base_addr_low; i < mmap->base_addr_low + mmap->length_low; i += PAGE_SIZE)
                {
                    //Indice dentro de mem_page_frames
                    uint32_t k = i/PAGE_SIZE;
                                    
                    if( i > kernel_physical_end + PAGE_SIZE - kernel_physical_end%(PAGE_SIZE))
                    {
                        //marcamos la pagina como libre    
                        mem_page_frames[k].ref_count = 0;
                        //Agregamos el frame al stack
                        mmu_push_free_frame(&mem_page_frames[k]);
                        //Incrementamos la cantidad de frames libres
                        free_page_frame_count++;
                    }
                    else
                    {
                        //estas paginas corresponden al kernel
                        mem_page_frames[k].ref_count = 1;
                        mem_page_frames[k].next      = NULL;
                        mem_page_frames[k].prev      = NULL;
                    }
                    //actualizamos la cantidad de frames totales en la memoria
                    page_frames_count = k+1;
                }
            }
        }
        
        //Ahora actualizamos los frames libres, porque free_page_frame_stack no fue considerado como parte del kernel
        kernel_physical_end += sizeof(page_frame_t)*page_frames_count;

        //Alineamos el kernel_physical_end a 4kb
        if(kernel_physical_end % PAGE_SIZE)
        {
            kernel_physical_end += (PAGE_SIZE - kernel_physical_end%PAGE_SIZE);
        }
    }
    else
    {
        //Si no, por ahora nos volvemos locos ya que hay que verificar la memoria "a mano"
        panic("Hubo un problema con la inicializacion de memoria: el memory map de grub no esta!");  
    }
    
    kprint("listo!  %dKB disponibles\n", available_physical_memory);
        
    //Ahora tenemos la lista de free_frames llena y conocemos la cantidad total de memoria fisica disponible
    //y sabemos cuantas paginas de 4mb ocupa el kernel, asi que podemos armar las tablas de paginacion!  
    mmu_init_paging(kernel_pdt);  
    //Finalmente, ahora podemos poner la gdt definitiva
    mmu_install_gdt();

	/* Marcamos las páginas como utilizadas */
	kprint( "Marcando desde 0x%x hasta 0x%x\n", fa, la );
	for ( i = fa; i < la; i += PAGE_SIZE ) {
		mmu_get_page_frame( KVA2PA(i) );
		kprint( "Marcando 0x%x\n", KVA2PA(i) );
	}
}

static void mmu_init_paging(uint32_t *kpdt)
{
    ///Limpiamos la pdt del kernel inicializada en 0
    memset(kpdt, 0, PAGE_SIZE);
    
    //Vamos a mapear toda la memoria fisica disponible a partir de 0x80...0
    //Cantidad de frames de tamaño 4mb
    uint32_t large_frames_count = page_frames_count / 1024;
    //Cantidad de frames de 4kb que sobran (siempre menor a 1024)
    uint32_t small_frames_count = page_frames_count % 1024;
    //Primero mapeamos todos los frames de 4mb
    uint32_t i;
    for(i=0; i<large_frames_count; i++)
    {
        kpdt[i]       = (i << 22) | PAGE_PRESENT | PAGE_SUPERVISOR | PAGE_RW | PAGE_4MB; 
        kpdt[i+512]   = (i << 22) | PAGE_PRESENT | PAGE_SUPERVISOR | PAGE_RW | PAGE_4MB;
    }
    //Si quedan frames por mapear, lo hacemos en una page table adicional (una para kernel y otra para user)
    uint32_t kernel_pte_pa = kernel_physical_end;
    uint32_t user_pte_pa   = kernel_physical_end + PAGE_SIZE;
    pde_t *kernel_pte      = (pde_t*)PA2KVA(kernel_pte_pa);
    pde_t *user_pte        = (pde_t*)PA2KVA(user_pte_pa);
    
    //Mapeamos la nueva ptable
    kpdt[i]     = user_pte_pa | PAGE_PRESENT | PAGE_SUPERVISOR | PAGE_RW;
    kpdt[i+512] = kernel_pte_pa | PAGE_PRESENT | PAGE_SUPERVISOR | PAGE_RW;
    
    //Ahora mapeamos el resto
    i = 0;
    while(i < small_frames_count)
    {
        kernel_pte[i]   = (large_frames_count * LARGE_PAGE_SIZE + i*PAGE_SIZE) | PAGE_PRESENT | PAGE_SUPERVISOR | PAGE_RW;
        user_pte[i]     = (large_frames_count * LARGE_PAGE_SIZE + i*PAGE_SIZE) | PAGE_PRESENT | PAGE_SUPERVISOR | PAGE_RW;
        i++;
    }
    
    //Activamos PSE (habilitamos paginacion con pag de 4mb y de 4kb)
    setCR4(getCR4() | 0x00000010);
    //Seteamos el cr3
    setCR3(KVA2PA((reg_t)kernel_pdt));
    //Leemos cr0
    setCR0(getCR0() | 0x80000000); 
    //Listo! Paginacion activada  
}

static void mmu_push_free_frame(page_frame_t *page_frame)
{
    if(free_page_frame_stack == NULL){
        //El stack esta vacio
        free_page_frame_stack = page_frame;
        page_frame->next = NULL;
        page_frame->prev = NULL;
    }
    else
    {
        //Reconectamos los punteros de la lista doblemente enlazada
        page_frame->next = free_page_frame_stack;
        page_frame->prev = NULL;
        page_frame->next->prev = page_frame;
        free_page_frame_stack = page_frame;
    }
    //Incrementamos la cantidad de frames libres
    free_page_frame_count++;
}

static page_frame_t* mmu_pop_free_frame()
{
    page_frame_t *frame = NULL;
    //Veamos si hay algun frame en el stack
    if(free_page_frame_stack != NULL)
    {
        frame = free_page_frame_stack;
        free_page_frame_stack = frame->next;
        free_page_frame_stack->prev = NULL;
        frame->next = NULL;

        free_page_frame_count--;
    }

    return frame;    
}

static uint32_t mmu_page_frame_2_PA(page_frame_t* page_frame)
{
    return (((uint32_t)page_frame - (uint32_t)mem_page_frames) / sizeof(page_frame_t)) * PAGE_SIZE;
}

static page_frame_t* mmu_PA_2_page_frame(uint32_t physical_address)
{
    uint32_t k = physical_address / PAGE_SIZE;
    if( k < page_frames_count)
    {
        return &mem_page_frames[k];
    }
    else
    {
        return NULL;
    }
}

static page_frame_t* mmu_get_page_frame( uint32_t pa )
{
    page_frame_t* frame = mmu_PA_2_page_frame( pa );
    if( frame )
    {
        //veo si estaba en la lista, para decrementar la cantidad de frames libres
        if( frame->next != NULL || frame->prev != NULL)
        {
            free_page_frame_count--;
        }   
        if( frame->next != NULL )
        {
            frame->next->prev = frame->prev;
        }       
        if( frame->prev != NULL )
        {
            frame->prev->next = frame->next;
        }
        else
        {
            //Puede ser el primero de la lista
            if( free_page_frame_stack == frame )
            {
                //era el primero
                free_page_frame_stack = frame->next;
            }
        }
        //anulo los punteros
        frame->next = NULL;
        frame->prev = NULL;
        frame->ref_count++;
    }
    return frame;
}

static uint8_t mmu_alloc_page(pde_t *pdt, page_frame_t *page_frame, uint32_t va, uint8_t perm, uint8_t force_dealloc)
{
    uint32_t pd_offset = GET_PD_OFFSET(va);
    uint32_t pt_offset = GET_PT_OFFSET(va);
    
    pte_t *pte = mmu_get_pte(pdt, va);
    
    if(pte && IS_PRESENT(*pte))
    {
        //Ya hay un mapeo para esa va
        if( force_dealloc)
        {
            page_frame_t *old_frame = (page_frame_t*)PA2KVA(mmu_PA_2_page_frame(GET_BASE_ADDRESS(*pte)));
            mmu_free_page_frame(old_frame);
        }
        else
        {
            return E_MMU_INVALID_VA;
        }
    }
    
    if(mmu_dirwalk(pdt, va, &pte, 1, IS_USER_PAGE(perm)) == E_MMU_SUCCESS)
    {
        //Incrementamos la cantidad de referencias
        page_frame->ref_count++;
        *pte = mmu_page_frame_2_PA(page_frame) | perm | PAGE_PRESENT;
		  if ((uint32_t)pdt == getCR3()) invlpg(va);

        return E_MMU_SUCCESS;
    }
    else
    {
        return E_MMU_NO_MEMORY;
    }
}

static void mmu_free_page_frame(page_frame_t *frame)
{
    frame->ref_count--;
    if(frame->ref_count == 0)
    {
        mmu_push_free_frame(frame);
    }
}

static uint8_t mmu_dirwalk(pde_t *pdt, uint32_t va, pte_t **pte, uint8_t create_page_table, uint8_t ptable_domain)
{
    uint32_t pd_offset = GET_PD_OFFSET(va);
    uint32_t pt_offset = GET_PT_OFFSET(va);
	
	 /* Con esto nos aseguramos que el usuario pueda
	  * accederlo, sólo si el pte tiene los permisos
	  * adecuados.
	  */
	// pdt[pd_offset] |= PAGE_USER | PAGE_RW;
    *pte = mmu_get_pte(pdt, va);
    if(*pte != NULL)
    {
        return E_MMU_SUCCESS;
    }
    else
    {
        //La page table no existe !
        if(create_page_table){
            //Ok, intentamos crearla          
            page_frame_t *new_frame = mmu_pop_free_frame();
            if(new_frame != NULL)
            {
                //Incrementamos la cantidad de referencias
                new_frame->ref_count++;
                //Obtenemos la direccion fisica del nuevo page_frame
                uint32_t new_frame_pa = mmu_page_frame_2_PA(new_frame);
                //Obtenemos un puntero al frame para llenarlo
                pte_t *new_ptable = (pte_t*)PA2KVA(new_frame_pa);
                //Inicializamos en 0 la nueva tabla de paginas
                memset(new_ptable, 0, PAGE_SIZE);
                //Realizamos el mapeo correspondiente
                if(ptable_domain != PAGE_USER && ptable_domain!=PAGE_SUPERVISOR)
                {
                    //por default es PAGE_SUPERVISOR
                    ptable_domain = PAGE_SUPERVISOR;
                }
                pdt[pd_offset] = new_frame_pa | ptable_domain | PAGE_RW | PAGE_PRESENT;
                //Llenamos pte
                *pte = &(new_ptable[pt_offset]);
                //Devolvemos SUCCESS !
                return E_MMU_SUCCESS;   
            }
            else
            {
                return E_MMU_NO_MEMORY;
            }
        }
        else
        {
            return E_MMU_PTABLE_NOT_PRESENT;
        }
    }
    /*
    if(IS_PRESENT(pdt[pd_offset]))
    {
        //Buenisimo, la pagina estaba presente asi que devolvemos la pte
        pte_t *page_table = (pte_t*)PA2KVA(GET_BASE_ADDRESS(pdt[pd_offset]));      
        *pte = &page_table[pt_offset];
        return E_MMU_SUCCESS;
    }    
    else
    {
        //La page table no existe !
        if(create_page_table){
            //Ok, intentamos crearla          
            page_frame_t *new_frame = mmu_pop_free_frame();
            if(new_frame != NULL)
            {
                //Incrementamos la cantidad de referencias
                new_frame->ref_count++;
                //Obtenemos la direccion fisica del nuevo page_frame
                uint32_t new_frame_pa = mmu_page_frame_2_PA(new_frame);
                //Obtenemos un puntero al frame para llenarlo
                pte_t *new_ptable = (pte_t*)PA2KVA(new_frame_pa);
                //Inicializamos en 0 la nueva tabla de paginas
                memset(new_ptable, 0, PAGE_SIZE);
                //Realizamos el mapeo correspondiente
                pdt[pd_offset] = new_frame_pa | PAGE_SUPERVISOR | PAGE_RW | PAGE_PRESENT;
                //Llenamos pte
                *pte = &(new_ptable[pt_offset]);
                //Devolvemos SUCCESS !
                return E_MMU_SUCCESS;   
            }
            else
            {
                return E_MMU_NO_MEMORY;
            }
        }
        else
        {
            return E_MMU_PTABLE_NOT_PRESENT;
        }
    }
    */
}

static pte_t* mmu_get_pte(pde_t *pdt, uint32_t va)
{
    uint32_t pd_offset = GET_PD_OFFSET(va);
    uint32_t pt_offset = GET_PT_OFFSET(va);
    if(IS_PRESENT(pdt[pd_offset]))
    {
        pte_t *page_table  = (pte_t*)PA2KVA(GET_BASE_ADDRESS(pdt[pd_offset]));
        return &page_table[pt_offset];   
    }
    else
    {
        return NULL;
    }
}

//Obtiene un page frame y lo mapea en va.
uint8_t mmu_alloc_at_VA(pde_t *pdt, uint32_t va, uint8_t perm, uint8_t force_dealloc )
{
    page_frame_t *free_frame = mmu_pop_free_frame();
    if(free_frame != NULL)
    {
        return mmu_alloc_page(pdt, free_frame, va, perm, force_dealloc);
    }
    else
    {
        return E_MMU_NO_MEMORY;
    }
}

uint8_t mmu_free(pde_t *pdt, uint32_t va )
{
    //Primero obtenemos la pte para obtener la direccion fisica del frame
    pte_t *pte;
    if(mmu_dirwalk(pdt, va, &pte, 0, 0) == E_MMU_SUCCESS)
    {
        page_frame_t *frame = mmu_PA_2_page_frame((uint32_t)GET_BASE_ADDRESS(*pte));
        mmu_free_page_frame(frame);
        //Invalido la va en la TLB
        //OJO: invalida la va de la PDT que esta instalada en este momento, cr3!
        invlpg(va);
        //Limpio la pte
        *pte = 0;
        
        return E_MMU_SUCCESS;
    }
    else
    {
        return E_MMU_INVALID_VA;
    }
}

uint8_t mmu_map_pa2va(pde_t *pdt, uint32_t pa, uint32_t va, uint8_t perm, uint8_t force_dealloc )
{
    //Obtengo el page_frame (estuviera libre o no, puede ser que varias va esten mapeadas a un mismo frame)
    page_frame_t *frame = mmu_get_page_frame(pa);
    
    return mmu_alloc_page(pdt, frame, va, perm, force_dealloc);
}

uint32_t mmu_get_free_frame_count()
{
    return free_page_frame_count;
} 

uint8_t mmu_alloc(uint32_t *pdt, uint32_t *va, uint32_t *pa, uint8_t perm)
{
    //encontremos una va libre
    uint32_t i = (IS_USER_PAGE(perm))? 0 : (KERNEL_VIRTUAL_START - KERNEL_PHYSICAL_START)/LARGE_PAGE_SIZE;
    uint8_t free_va = false;
    
    while(i<1024 && !free_va)
    {
        pde_t pde = pdt[i];
        if(IS_PRESENT(pde) && !IS_4MB(pde))
        {
            uint32_t j = 0;
            pte_t *ptable = (pte_t*)PA2KVA(GET_BASE_ADDRESS(pde));
            while(j<1024 && !free_va)
            {
                pte_t pte = ptable[j];
                if(!IS_PRESENT(pte))
                {
                    *va = i<<22 | j<<12;
                    free_va = true;
                }
                j++;
            }
        }
        else
        {
            if(!IS_4MB(pde))
            {
                *va = i<<22;
                free_va = true;
            }
        }
        i++;
    }

    if(free_va)
    {
        //si pudimos encontrar una va libre
        page_frame_t *free_frame = mmu_pop_free_frame();
        if(free_frame != NULL)
        {
            *pa = mmu_page_frame_2_PA(free_frame);
            return mmu_alloc_page(pdt, free_frame, *va, perm, IS_USER_PAGE(perm));
        }
        else
        {
            return E_MMU_NO_MEMORY;
        }
    }    
    else
    {
        return E_MMU_NO_MEMORY;
    }
}

static void mmu_install_gdt()
{
	// Inicializamos la GDT.
	gdtr_t *gdtr = (gdtr_t *) g_GDT;
	gdtr->limit = (uint16_t) (sizeof( g_GDT ) - 1);
	gdtr->base = (uint32_t)g_GDT; // Dirección física.
	
	gdt_fill_code_segment( g_GDT + 1, (void *) 0, 0xFFFFFFFF, 0 ); // Código kernel
	gdt_fill_data_segment( g_GDT + 2, (void *) 0, 0xFFFFFFFF, 0 ); // Datos kernel
	gdt_fill_code_segment( g_GDT + 3, (void *) 0, 0xFFFFFFFF, 3 ); // Código usuario
	gdt_fill_data_segment( g_GDT + 4, (void *) 0, 0xFFFFFFFF, 3 ); // Datos usuario
	// Por lo que entiendo, LGDT puede leer una dirección virtual.
	lgdt( g_GDT ); // Dirección física
}

uint8_t mmu_install_task_pdt(uint32_t *va, uint32_t *pa)
{
    uint32_t task_pdt_va, task_pdt_pa;
    if(mmu_alloc(kernel_pdt, &task_pdt_va, &task_pdt_pa, PAGE_SUPERVISOR|PAGE_PRESENT|PAGE_RW)==E_MMU_SUCCESS)
    { 
        pde_t *pdt = (pde_t*) task_pdt_va;
        memset(pdt, 0, PAGE_SIZE);
        
        //Copiamos el mapeo del kernel para que la tarea pueda ver y usar el kernel
        uint32_t i;
        for(i=0; i<512; i++)
        {
            pdt[i+512] = kernel_pdt[i+512];
        }

        *va = task_pdt_va;
        *pa = task_pdt_pa;
        
        return E_MMU_SUCCESS;
    }
    else
    {
        return E_MMU_NO_MEMORY;
    }
}

uint8_t mmu_kalloc( uint32_t *va ) {
	uint32_t pa;

	page_frame_t *free_frame = mmu_pop_free_frame();
	if (!free_frame) return E_MMU_NO_MEMORY;
	free_frame->ref_count++;
	
	pa = mmu_page_frame_2_PA(free_frame);
	*va = PA2KVA(pa);
	return E_MMU_SUCCESS;
}
