#include <screen/screen.h>
#include <asm/asm.h>
#include <asm/gdt.h>
#include <mem/mmu.h>
#include <kernel/globals.h>
#include <kernel/panic.h>
#include <lib/string.h>

//Instala la gdt definitiva, reemplazando a la dummy_gdt
void install_gdt();

//puntero al final del codigo del kernel (provisto por el linker)
extern uint8_t __kernel_end[];

static uint32_t kernel_physical_end = KVA2PA((uint32_t)__kernel_end);

//total de memoria fisica disponible para administrar por el SO (es menor que la total)
static uint32_t available_physical_memory = 0;

//mem_frames es un arreglo de page_frame_t. Basicamente, permite encontrar en O(1) si un frame tiene referencias externas
static page_frame_t* mem_page_frames = NULL;

//stack de frames libres
static page_frame_t* free_page_frame_stack = NULL;

//puntero al final de free_frames para poder usarla como una cola (no se porque entonces no la llame 'cola' xD)
//static page_frame_t* free_pageframes_end;

//cantidad de frames en la memoria fisica disponible
static uint32_t page_frames_count = 0;

//cantidad de frames libres en la memoria fisica
static uint32_t free_page_frame_count = 0;

//la Page Directory Table del kernel
static pde_t kernel_pdt[1024]  __attribute__ ((aligned (4096)));

//Inicializa las estructuras relacionadas con la admin de memoria
void init_mem(multiboot_info_t* mbd)
{  
    if(mbd->flags & 0x0001)
    { 
        available_physical_memory = mbd->mem_lower + mbd->mem_upper;
    }
    
    //El array de page_frame_t empieza justo al final del codigo del kernel
    mem_page_frames = (page_frame_t*)(PA2KVA(kernel_physical_end));
    
    //inicializamos la cantidad de paginas de KERNEL_PAGESIZE que ocupa el kernel
    uint32_t kernel_pages_count = 0;
    
    //Si tenemos disponible el mapa de memoria del GRUB, usarlo!
    if(mbd->flags & 0x0020)
    {      
        memory_map_t *mmap;

        kprint("Checking RAM...");
        for(mmap = (memory_map_t *) (mbd->mmap_addr + 0x80000000);
            (unsigned long) mmap < (mbd->mmap_addr + mbd->mmap_length + 0x80000000);
            mmap = (memory_map_t *) ((unsigned long) mmap
                                     + mmap->size + sizeof (mmap->size)))
        {
            kprint("  Size = 0x%x, Base_Address = 0x%x,"
                   " Length = 0x%x, Type = 0x%x\n",
                   mmap->size,
                   mmap->base_addr_low,
                   mmap->length_low,
                   mmap->type);

            //Solo vamos a administrar las partes reservadas para el SO
            //El resto esta reservado para el bios y otras cosas.
            if(mmap->type == 0x01)
            {
                //Inicializamos la lista de frames libres y el arreglo de referencias
                uint32_t i;
                for(i = mmap->base_addr_low; i < mmap->base_addr_low + mmap->length_low; i += PAGESIZE)
                {
                    //Indice dentro de mem_page_frames
                    uint32_t k = i/PAGESIZE;
                                    
                    if( i > kernel_physical_end + KERNEL_PAGESIZE - kernel_physical_end%(KERNEL_PAGESIZE))
                    {
                        //marcamos la pagina como libre    
                        mem_page_frames[k].ref_count = 0;
                        //Agregamos el frame al stack
                        push_free_frame(&mem_page_frames[k]);
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

        //Calculamos la cantidad de paginas de 4mb para meter todo el kernel dentro de ellas
        kernel_pages_count  = kernel_physical_end / KERNEL_PAGESIZE;
        if(kernel_physical_end % KERNEL_PAGESIZE)
        {
            kernel_pages_count++;
            //Ademas, marcamos como utilizadas a los frames que sean necesarios para completar la pag de 4mb
            uint32_t k;
            //"k" es la pagina de 4kb donde termina el kernel. Marcamos todas las paginas que faltan
            //desde la k-esima hasta que k sea multiplo de 1024. Ahi terminamos porque hemos marcado
            //la cantidad de paginas de 4kb para completar una pagina de 4mb
            //Estos frames van a quedar como utilizados siempre porque son donde reside el kernel
            //que nunca se desaloja.
            for(k = (kernel_physical_end / PAGESIZE)+1; k%1024 ; k++)
            {
                mem_page_frames[k].ref_count = 1;
            }
            //alineamos el final del kernel a 4mb
            kernel_physical_end += KERNEL_PAGESIZE + (kernel_physical_end % KERNEL_PAGESIZE);            
        }
    }
    else
    {
        //Si no, por ahora nos volvemos locos ya que hay que verificar la memoria "a mano"
        panic("Hubo un problema con la inicializacion de memoria: el memory map de grub no esta!");  
    }
    
    kprint("done!  %dKB available\n", available_physical_memory);
        
    //Ahora tenemos la lista de free_frames llena y conocemos la cantidad total de memoria fisica disponible
    //y sabemos cuantas paginas de 4mb ocupa el kernel, asi que podemos armar las tablas de paginacion!  
    init_paging(kernel_pages_count);  
    //finalmente, ahora podemos poner la gdt definitiva
    install_gdt();
    //incializamos el espacio para el heap del kernel
    page_alloc_at_VA( kernel_pdt, KERNEL_HEAP_START, PAGE_RW | PAGE_SUPERVISOR, 1 ); //force allocation
}

void init_paging(uint32_t kernel_pages_count)
{
    memset(kernel_pdt, 0, PAGESIZE);
    
    //Mapeamos el kernel desde 0 hasta 4mb*kernel_pages_count
    //Y tambien desde 2gb hasta 2gb+4mb*kernel_pages_count para que al inicializar
    //paginacion y luego cambiar la dummy_gdt por la final, funcione todo correctamente
    uint32_t i;
    for(i=0; i<kernel_pages_count; i++)
    {
        kernel_pdt[i]       = (i << 22) | 0x83; 
        kernel_pdt[i+512]   = (i << 22) | 0x83;
    }

    //armamos la tabla para mapear las futuras tablas de paginacion y asi tenerlas en direcciones separadas
    page_frame_t *frame = pop_free_frame();
    uint32_t frame_pa = get_page_frame_PA(frame);
    memset((pde_t*)PA2KVA(frame_pa), 0, PAGESIZE);
    
    //Apuntamos a la page table
    kernel_pdt[GET_PD_OFFSET(KERNEL_PAGING_TABLES_VA)] = frame_pa | PAGE_SUPERVISOR | PAGE_RW | PAGE_PRESENT; 
    pte_t *pte = (pte_t*)PA2KVA(frame_pa);
    pte[0] = frame_pa | PAGE_SUPERVISOR | PAGE_RW | PAGE_PRESENT; 
    
    //activamos PSE (habilitamos paginacion con pag de 4mb y de 4kb)
    setCR4(getCR4() | 0x00000010);
    //seteamos el cr3
    setCR3(KVA2PA((reg_t)kernel_pdt));
    //leemos cr0
    setCR0(getCR0() | 0x80000000); //listo!  
}

//Agrega page_frame al stack de frames libres. Debe ser llamado cuando ref_count==0
void push_free_frame(page_frame_t *page_frame)
{
    if(free_page_frame_stack == NULL){
        //El stack esta vacio
        free_page_frame_stack = page_frame;
        page_frame->next = NULL;
        page_frame->prev = NULL;
    }
    else
    {
        //"Reconectamos" los punteros de la lista doblemente enlazada
        page_frame->next = free_page_frame_stack;
        page_frame->prev = NULL;
        page_frame->next->prev = page_frame;
        free_page_frame_stack = page_frame;
    }
    free_page_frame_count++;
}

page_frame_t* pop_free_frame()
{
    page_frame_t *frame = NULL;
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

uint32_t get_page_frame_PA(page_frame_t* frame)
{
    return (((uint32_t)frame - (uint32_t)mem_page_frames) / sizeof(page_frame_t)) * PAGESIZE;
}

page_frame_t* get_PA_page_frame(uint32_t physical_address)
{
    uint32_t k = physical_address / PAGESIZE;
    if( k < page_frames_count)
    {
        return &mem_page_frames[k];
    }
    else
    {
        return NULL;
    }
}

page_frame_t* get_page_frame( uint32_t pa )
{
    page_frame_t* frame = get_PA_page_frame( pa );
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

int8_t page_alloc(pde_t *pdt, page_frame_t *page_frame, uint32_t va, uint8_t perm, uint8_t force_dealloc)
{
    uint32_t pd_offset = GET_PD_OFFSET(va);
    uint32_t pt_offset = GET_PT_OFFSET(va);
    
    pte_t *pte = NULL;
    
    if(page_dirwalk(pdt, va, &pte, 1) == E_MMU_SUCCESS)
    {
        //El if que sigue es ensalada de fruta. Corregir para hacer un dealloc correcto.
        /*
        if(*pte != 0)
        {
            //Ya habia un frame asociado a "va"
            if( force_dealloc)
            {
                page_frame_t *old_frame = PA2KVA(get_PA_page_frame(GET_BASE_ADDRESS(*pte)));
                page_dealloc(old_frame);
            }
            else
            {
                return E_MMU_INVALID_VA;
            }
        }
        */
        //Incrementamos la cantidad de referencias
        page_frame->ref_count++;
        *pte = get_page_frame_PA(page_frame) | perm | PAGE_PRESENT;
		  if (pdt == getCR3()) invlpg(va);

        return E_MMU_SUCCESS;
    }
    else
    {
        return E_MMU_NO_MEMORY;
    }
}

void page_dealloc(page_frame_t *frame)
{
    frame->ref_count--;
    if(frame->ref_count == 0)
    {
        push_free_frame(frame);
    }
}

pte_t* page_install_page_table(pde_t *pdt, page_frame_t *frame)
{
    static uint32_t i = 1;

    uint32_t ptable_pa = get_page_frame_PA(frame);
    uint32_t ptable_va;
    //obtengo el puntero a la tabla de paginas que mapea a todas las demas (incluida ella misma)
    pte_t *master_page_table = (pte_t*)KERNEL_PAGING_TABLES_VA;
    //mapeo la nueva tabla de pagina
    master_page_table[i] = ptable_pa | PAGE_SUPERVISOR | PAGE_RW | PAGE_PRESENT;
    ptable_va = KERNEL_PAGING_TABLES_VA + (i++ << 12);
    //Ahora que esta mapeada, puedo usar un puntero para limpiarla
	 invlpg(ptable_va);
    memset((pde_t*)ptable_va, 0, PAGESIZE);
    //devolvemos el puntero
    return (pde_t*)ptable_va;
}

pte_t *get_page_table_va(uint32_t page_table_pa)
{
    pte_t *master_page_table = (pte_t*)KERNEL_PAGING_TABLES_VA;
    uint32_t i;
    for(i=1; i<1024 ;i++)
    {
        if(GET_BASE_ADDRESS(master_page_table[i]) == page_table_pa)
        {
            uint32_t pte = KERNEL_PAGING_TABLES_VA + (i++ << 12);
            return (pte_t *)pte;
        }
    }
    
    return NULL;
}

int8_t page_dirwalk(pde_t *pdt, uint32_t va, pte_t **pte, uint8_t create_page_table)
{
    uint32_t pd_offset = GET_PD_OFFSET(va);
    uint32_t pt_offset = GET_PT_OFFSET(va);    
    
    if(IS_PRESENT(pdt[pd_offset]))
    {
        pte_t *page_table = get_page_table_va(GET_BASE_ADDRESS(pdt[pd_offset]));      
        *pte = &page_table[pt_offset];
        return E_MMU_SUCCESS;
    }    
    else
    {
        //La page table no existe !
        if(create_page_table){
            //Ok, intentamos crearla          
            page_frame_t* new_frame = pop_free_frame();
            if(new_frame != NULL)
            {
                new_frame->ref_count++;
                pte_t *new_page_table = page_install_page_table(pdt, new_frame);
                
                pdt[pd_offset] = get_page_frame_PA(new_frame) | PAGE_SUPERVISOR | PAGE_RW | PAGE_PRESENT;
                
                *pte = &(new_page_table[pt_offset]);
                
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
}

//Obtiene un page frame y lo mapea en va.
uint8_t page_alloc_at_VA(pde_t *pdt, uint32_t va, uint8_t perm, uint8_t force_dealloc )
{
    page_frame_t *free_frame = pop_free_frame();
    if(free_frame != NULL)
    {
        return page_alloc(pdt, free_frame, va, perm, force_dealloc);
    }
    else
    {
        return E_MMU_NO_MEMORY;
    }
}

uint8_t page_free(pde_t *pdt, uint32_t va )
{
    //Primero obtenemos la pte para obtener la direccion fisica del frame
    pte_t *pte;
    if(page_dirwalk(pdt, va, &pte, 0) == E_MMU_SUCCESS)
    {
        page_frame_t *frame = get_PA_page_frame((uint32_t)GET_BASE_ADDRESS(*pte));
        page_dealloc(frame);
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

uint8_t page_map_pa2va(pde_t *pdt, uint32_t pa, uint32_t va, uint8_t perm, uint8_t force_dealloc )
{
    //Obtengo el page_frame (estuviera libre o no, puede ser que varias va esten mapeadas a un mismo frame)
    page_frame_t *frame = get_page_frame(pa);
    
    return page_alloc(pdt, frame, va, perm, force_dealloc);
}

uint32_t get_free_page_frame_count()
{
    return free_page_frame_count;
} 

uint8_t mmu_alloc(uint32_t *pdt, uint32_t *va, uint8_t perm, uint32_t *pa)
{
    //encontremos una va libre
    uint32_t i = (IS_USER_PAGE(perm))? 0 : (KERNEL_VIRTUAL_START - KERNEL_PHYSICAL_START)/KERNEL_PAGESIZE;
    uint8_t free_va = false;
    
    while(i<1024 && !free_va)
    {
        pde_t pde = pdt[i];
        if(IS_PRESENT(pde) && !IS_4MB(pde))
        {
            uint32_t j = 0;
            pte_t *ptable = get_page_table_va(GET_BASE_ADDRESS(pde));
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
        page_frame_t *free_frame = pop_free_frame();
        if(free_frame != NULL)
        {
            *pa = get_page_frame_PA(free_frame);
            return page_alloc(pdt, free_frame, *va, perm, 0);
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

void install_gdt()
{
	// Inicializamos la GDT.
	gdtr_t *gdtr = (gdtr_t *) g_GDT;
	gdtr->limit = (uint16_t) (sizeof( g_GDT ) - 1);
	gdtr->base = (uint32_t)KVA2PA(g_GDT); // Dirección física.
	
	kprint("GDT BASE: 0x%x (0x%x)\n", (uint32_t) g_GDT, (uint32_t) KVA2PA((uint32_t) g_GDT) );

	gdt_fill_code_segment( g_GDT + 1, (void *) 0, 0xFFFFFFFF, 0 ); // Código kernel
	gdt_fill_data_segment( g_GDT + 2, (void *) 0, 0xFFFFFFFF, 0 ); // Datos kernel
	gdt_fill_code_segment( g_GDT + 3, (void *) 0, 0xFFFFFFFF, 3 ); // Código usuario
	gdt_fill_data_segment( g_GDT + 4, (void *) 0, 0xFFFFFFFF, 3 ); // Datos usuario
	// Por lo que entiendo, LGDT puede leer una dirección virtual.
	lgdt( (void *) ((unsigned long) KVA2PA(g_GDT)) ); // Dirección física
}

uint8_t install_task_pdt(uint32_t *va, uint32_t *pa)
{
    uint32_t task_pdt_va, task_pdt_pa;
    if(mmu_alloc(kernel_pdt, &task_pdt_va, PAGE_SUPERVISOR|PAGE_PRESENT|PAGE_RW, &task_pdt_pa)==E_MMU_SUCCESS)
    { 
        memset((pde_t*)task_pdt_va, 0, PAGESIZE);
        
        //Mapeamos el kernel desde 0 hasta 4mb*kernel_pages_count
        //Y tambien desde 2gb hasta 2gb+4mb*kernel_pages_count para que al inicializar
        //paginacion y luego cambiar la dummy_gdt por la final, funcione todo correctamente
        uint32_t i;
        uint32_t kernel_pages_count = kernel_physical_end / KERNEL_PAGESIZE;
        if(kernel_physical_end % KERNEL_PAGESIZE)
        {
            kernel_pages_count++;
        }
        
        for(i=0; i<kernel_pages_count; i++)
        {
            ((pde_t*)task_pdt_va)[i+512]   = ((i*KERNEL_PAGESIZE) << 12) | PAGE_4MB | PAGE_PRESENT | PAGE_SUPERVISOR | PAGE_RW;
        }

        //armamos la tabla para mapear las futuras tablas de paginacion y asi tenerlas en direcciones separadas
        uint32_t task_master_ptable_va, task_master_ptable_pa;
        mmu_alloc(kernel_pdt, &task_master_ptable_va, PAGE_SUPERVISOR|PAGE_PRESENT|PAGE_RW, &task_master_ptable_pa);
        memset((pde_t*)task_master_ptable_va, 0, PAGESIZE);
        
        //Apuntamos a la task_master_ptable
        ((pde_t*)task_pdt_va)[GET_PD_OFFSET(KERNEL_PAGING_TABLES_VA)] = task_master_ptable_pa | PAGE_SUPERVISOR | PAGE_RW | PAGE_PRESENT; 
        pte_t *pte = (pte_t*)task_master_ptable_va;
        pte[0] = task_master_ptable_pa | PAGE_SUPERVISOR | PAGE_RW | PAGE_PRESENT; 
        
        *va = task_pdt_va;
        *pa = task_pdt_pa;
        
        return E_MMU_SUCCESS;
    }
    else
    {
        return E_MMU_NO_MEMORY;
    }
}
