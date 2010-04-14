#ifndef __MMU__H__
#define __MMU__H__

#include <asm/types.h>
#include <boot/multiboot.h>
#include <mem/memlayout.h>

#define NULL  0

//Macros para obtener los offsets correspondiendentes de una virtual address
#define GET_PD_OFFSET(x)  (((x) & 0xFFC00000) >> 22)
#define GET_PT_OFFSET(x)  (((x) & 0x003FF000) >> 12)

//Permisos comunes entre los PDE Y PTE
#define PAGE_PRESENT    0x01
#define PAGE_RW         0X02
#define PAGE_READONLY   0x00
#define PAGE_USER       0x04
#define PAGE_SUPERVISOR 0x00
#define PAGE_WTHROUGH   0X08
#define PAGE_CACHEDIS   0X16
#define PAGE_4MB        0x80

//Macros para analizar los permisos y direccioens de las PDE y PTE
#define IS_PRESENT(x)       ((x) & PAGE_PRESENT)
#define IS_READWRITE(x)     ((x) & PAGE_RW)
#define IS_USER_PAGE(x)     ((x) & PAGE_USER)
#define IS_WRITETHROUGH(x)  ((x) & PAGE_WTHROUGH)
#define IS_CACHEDISABLED(x) ((x) & PAGE_CACHEDIS)
#define IS_4MB(x)           ((x) & PAGE_4MB)

#define GET_BASE_ADDRESS(x) ((x) & 0xFFFFF000)

//Codigo de errores generales de las funcioens de la MMU
#define E_MMU_SUCCESS               0X01
#define E_MMU_NO_MEMORY             0x02
#define E_MMU_PTABLE_NOT_PRESENT    0x03
#define E_MMU_INVALID_VA            0x04

typedef struct page_frame
{
   struct page_frame* next; //proximo frame de la lista de frames libres
   struct page_frame* prev; //frame anterior de la lista de frames libres
   uint32_t ref_count;  //cantidad de referencias externas
} page_frame_t;

//page directory table entry
typedef uint32_t pde_t; 
//page table entry
typedef uint32_t pte_t;

//Inicializa las estructuras relacionadas con la administracion
//de memoria, instala una gdt definitiva, y activa paginacion
void init_mem(multiboot_info_t* mbd);

//Inicializa las PDT en 0 salvo las entradas que corresponden
//al kernel. El kernel se guarda en paginas de KERNEL_PAGESIZE (que son de 4mb)
//kernel_pages_count: es la cantidad de paginas de 4mb que ocupa el kernel
void init_paging(uint32_t kernel_pages_count);

//Inicializa una parte de la memoria como heap para que sea usada por kmalloc
void init_kernel_heap();

//Libera el page_frame, devolviendolo al stack de frames libres
void push_free_frame(page_frame_t* page_frame);

//Devuelve un page_frame libre del stack, quitandolo de este
page_frame_t* pop_free_frame();

//Devuelve la direccion fisica del page_frame_t frame
uint32_t get_page_frame_PA(page_frame_t* frame);

//A partir de una direccion fisica, devuelve un puntero al page_frame_t correspondiente en el array mem_page_frames
//No quita al frame de la lista de libres si es que forma parte de dicha lista.
//Devuelve NULL si la direccion fisica va mas alla de la memoria fisica disponible
//NOTA: physical_address debe ser multiplo de PAGESIZE
page_frame_t* get_PA_page_frame(uint32_t physical_address);

//Devuelve el frame correspondiente a la physical address pasada como parametro, quitandolo de la lista de libres si es que estaba libre
//En caso de que la pa no sea válida, devuelve NULL
page_frame_t* get_page_frame( uint32_t physical_address );

//Asocia la direccion "va" con el frame "page_frame" en la tabla apuntada por pdt con los permisos "perm"
//Si la va ya estaba mapeada a otro lado, force_dealloc indica si se debe desalojar (==1) y remapear, o devolver error (==0)
//Valores de retorno:   ->E_MMU_SUCCESS: si se pudo asociar correctamente va con el page_frame
//                      ->E_MMU_INVALID_VA: si la va ya estaba mapeada a otro lado y force_dealloc==0
//                      ->E_MMU_NO_MEMEMORY: si no se pudo completar la operacion
int8_t page_alloc(pde_t *pdt, page_frame_t *page_frame, uint32_t va, uint8_t perm, uint8_t force_dealloc);

//Recorre las tablas y hace apuntar pte a la page table entry que corresponde a va dentro de pdt
//En caso de que todo salga bien, devuelve 1 y en pte el puntero a la pte correspondiente
//Si la page table no existe, create_page_table determina si se crea una nueva, o se devuelve error
//Posibles valores de retorno: -> E_MMU_SUCCESS: si todo salio bien
//                             -> E_MMU_PTABLE_NOT_PRESENT: si la tabla de paginas no existe y create_page_table==0
//                             -> E_MMU_NO_MEMORY: si la creacion de la tabla de paginas fallo
int8_t page_dirwalk(pde_t *pdt, uint32_t va, pte_t **pte, uint8_t create_page_table);

//Decrementa las referencias del page_frame, y si estas llegan a ser nulas, lo agrega al stack de frames libres
void page_dealloc(page_frame_t *frame);

//Obtiene un page frame y lo mapea en va.
//Valores de retorno:   ->E_MMU_SUCCESS: si todo salio bien
//                      ->E_MMU_NO_MEMORY: si no había más memoria fisica para realizar la operacion                      
uint8_t page_alloc_at_VA( pde_t* pdt, uint32_t va, uint8_t perm, uint8_t force_dealloc );

//A partir de la dirección virtual, obtiene el frame, le decrementa la cuenta de referencia y lo
//desmapea + invlpg().
//Valores de retorno:       ->E_MMU_SUCCESS: si todo sale bien
//                          ->E_MMU_INVALID_VA: si en realidad no habia nada en esa va, osea page_dirwalk devuelve que la ptable no esta
uint8_t page_free( pde_t *pdt, uint32_t va );

//Intenta asocia, para la pdt pasada como parámetro, la physical address con la virtual address. "force_dealloc" funciona igual que para page_alloc
//Valores de retorno:       ->E_MMU_SUCCESS: si todo sale bien
//                          ->E_MMU_INVALID_VA: si la virtual address ya esta mapeada a otra direccion fisica (liberarla primero en todo caso con page_free)
//                          ->E_MMU_NO_MEMORY: si no hay mas lugar en memoria fisica para realizar el mapeo
uint8_t page_map_pa2va(pde_t *pdt, uint32_t pa, uint32_t va, uint8_t perm, uint8_t force_dealloc );

//Instala una nueva tabla de paginas en el PDT correspondiente a la direccion virtual va usando el page_frame_t frame
//Devuelve el puntero a la nueva tabla de paginas. La tabla queda inicializada con 0's
pte_t* page_install_page_table(pde_t *pdt, page_frame_t *frame);

//Devuelve cuantos frames quedan libres en memoria fisica
uint32_t get_free_page_frame_count(); 

//Devuelve la direccion virtual de la tabla de paginas
pte_t *get_page_table_va(uint32_t page_table_pa);

//Instala una nueva pdt para una tarea
//Valores de retorno:   ->E_MMU_SUCCESS: si sale todo bien
//                      ->E_MMU_NO_MEMORY:  si no hay memoria para realizar la operacion
uint8_t install_task_pdt(uint32_t *va, uint32_t *pa);

//Toma un frame fisico libre y lo mapea en la primera direccion virtual libre disponible.
//En va devuelve la dirección virtual y en pa la dirección física.
//Valores de retorno:   ->E_MMU_SUCCESS
//                      ->E_MMU_NO_MEMORY
uint8_t mmu_alloc(uint32_t *pdt, uint32_t *va, uint8_t perm, uint32_t *pa);

#endif
