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
/*
            kprint("  Size = 0x%x, Base_Address = 0x%x,"
                   " Length = 0x%x, Type = 0x%x\n",
                   mmap->size,
                   mmap->base_addr_low,
                   mmap->length_low,
                   mmap->type);
*/            
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
}

void init_paging(uint32_t kernel_pages_count)
{
    memset(kernel_pdt, 0, 1024);
    
    //Mapeamos el kernel desde 0 hasta 4mb*kernel_pages_count
    //Y tambien desde 2gb hasta 2gb+4mb*kernel_pages_count para que al inicializar
    //paginacion y luego cambiar la dummy_gdt por la final, funcione todo correctamente
    uint32_t i;
    for(i=0; i<kernel_pages_count; i++)
    {
        kernel_pdt[i]       = ((i*KERNEL_PAGESIZE) << 12) | 0x83; 
        kernel_pdt[i+512]   = ((i*KERNEL_PAGESIZE) << 12) | 0x83;
    }

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
        page_frame->next = 0;
    }
    else
    {
        page_frame->next = free_page_frame_stack;
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
        frame->next = NULL;

        free_page_frame_count--;
    }

    return frame;    
}

uint32_t get_page_frame_KVA(page_frame_t* frame)
{
    return ((uint32_t)frame - (uint32_t)mem_page_frames) * PAGESIZE;
}

page_frame_t* get_page_frame_from_PA(uint32_t physical_address)
{
    return &mem_page_frames[physical_address / PAGESIZE];
}

int8_t page_alloc(pde_t *pdt, page_frame_t *page_frame, uint32_t va, uint8_t perm)
{
    uint32_t pd_offset = GET_PD_OFFSET(va);
    uint32_t pt_offset = GET_PT_OFFSET(va);
    
    pte_t *pte = NULL;
    
    if(page_dirwalk(pdt, va, &pte, 1) == E_SUCCESS)
    {
        if(*pte != 0)
        {
            //Ya habia un frame asociado a "va"
            page_frame_t *old_frame = get_page_frame_from_PA(GET_BASE_ADDRESS(*pte));
            page_dealloc(old_frame);
        }
        //Incrementamos la cantidad de referencias
        page_frame->ref_count++;
        *pte = get_page_frame_KVA(page_frame) | perm | PAGE_PRESENT;
    }
    else
    {
        return E_NO_MEMORY;
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

int8_t page_dirwalk(pde_t *pdt, uint32_t va, pte_t **pte, uint8_t create_page_table)
{
    uint32_t pd_offset = GET_PD_OFFSET(va);
    uint32_t pt_offset = GET_PT_OFFSET(va);    
    
    if(IS_PRESENT(pdt[pd_offset]))
    {
        //Bien! La page table esta presente
        pte_t *page_table = (pte_t*)GET_BASE_ADDRESS(pdt[pd_offset]);
        *pte = &page_table[pt_offset];
        return E_SUCCESS;
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
                
                pte_t *new_page_table = (pte_t*)KVA2PA(get_page_frame_KVA(new_frame));
                pdt[pd_offset] = (uint32_t)new_page_table | PAGE_PRESENT | PAGE_RW;
                
                //Inicializamos la nueva page table llena de 0's
                memset(new_page_table, 0, PAGESIZE);
                
                *pte = &new_page_table[pt_offset];
                
                return E_SUCCESS;   
            }
            else
            {
                return E_NO_MEMORY;
            }
        }
        else
        {
            return E_PTABLE_NOT_PRESENT;
        }
    }
}

//Obtiene un page frame y lo mapea en va.
uint8_t page_alloc_at_VA(pde_t *pdt, uint32_t va, uint8_t perm )
{
    page_frame_t *free_frame = pop_free_frame();
    if(free_frame != NULL)
    {
        return page_alloc(pdt, free_frame, va, perm);
    }
    else
    {
        return E_NO_MEMORY;
    }
}

void install_gdt()
{
	// Inicializamos la GDT.
	gdtr_t *gdtr = (gdtr_t *) g_GDT;
	gdtr->limit = (uint16_t) (sizeof( g_GDT ) - 1);
	gdtr->base = (uint32_t)KVA2PA(g_GDT); // Dirección física.

	gdt_fill_code_segment( g_GDT + 1, (void *) 0, 0xFFFFFFFF, 0 ); // Código kernel
	gdt_fill_data_segment( g_GDT + 2, (void *) 0, 0xFFFFFFFF, 0 ); // Datos kernel
	gdt_fill_code_segment( g_GDT + 3, (void *) 0, 0xFFFFFFFF, 3 ); // Código usuario
	gdt_fill_data_segment( g_GDT + 4, (void *) 0, 0xFFFFFFFF, 3 ); // Datos usuario
	// Por lo que entiendo, LGDT puede leer una dirección virtual.
	lgdt( (void *) ((unsigned long) KVA2PA(g_GDT)) ); // Dirección física
}
