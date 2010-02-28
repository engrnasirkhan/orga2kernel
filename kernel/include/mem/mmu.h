#ifndef __MMU__H__
#define __MMU__H__

#include <asm/types.h>
#include <boot/multiboot.h>
#include <mem/memlayout.h>

#define NULL  0

//Macros para obtener los offsets correspondiendentes de una virtual address
#define GET_PD_OFFSET(x)  ((x & 0xFFC00000) >> 22)
#define GET_PT_OFFSET(x)  ((x & 0x003FF000) >> 12)

//Permisos comunes entre los PDE Y PTE
#define PAGE_PRESENT    0x01
#define PAGE_RW         0X02
#define PAGE_USER       0x04
#define PAGE_WTHROUGH   0X08
#define PAGE_CACHEDIS   0X16

//Macros para analizar los permisos y direccioens de las PDE y PTE
#define IS_PRESENT(x)       (x & PAGE_PRESENT)
#define IS_READWRITE(x)     (x & PAGE_RW)
#define IS_USER_PAGE(x)     (x & PAGE_USER)
#define IS_WRITETHROUGH(x)  (x & PAGE_WTHROUGH)
#define IS_CACHEDISABLED(x) (x & PAGE_CACHEDIS)

#define GET_BASE_ADDRESS(x) ((x & 0xFFFFF000) >> 12)

//Codigo de errores generales de las funcioens de la MMU
#define E_SUCCESS               0X01
#define E_NO_MEMORY             0x02
#define E_PTABLE_NOT_PRESENT    0x03
#define E_INVALID_VA            0x04

typedef struct page_frame
{
   struct page_frame* next; //proximo frame de la lista de frames libres
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

//Libera el page_frame, devolviendolo al stack de frames libres
void push_free_frame(page_frame_t* page_frame);

//Devuelve un page_frame libre del stack, quitandolo de este
page_frame_t* pop_free_frame();

//Devuelve la direccion virtual del page_frame_t frame
uint32_t get_page_frame_KVA(page_frame_t* frame);

//A partir de una direccion fisica, devuelve un puntero al page_frame_t correspondiente en el array mem_page_frames
//Nota: physical_address deberia ser multiplo de PAGESIZE
page_frame_t* get_page_frame_from_PA(uint32_t physical_address);

//Asocia la direccion "va" con el frame "page_frame" en la tabla apuntada por pdt con los permisos "perm"
//Valores de retorno:   ->E_SUCCESS: si se pudo asociar correctamente va con el page_frame
//                      ->E_NO_MEMEMORY: si no se pudo completar la operacion
int8_t page_alloc(pde_t *pdt, page_frame_t *page_frame, uint32_t va, uint8_t perm);

//Recorre las tablas y hace apuntar pte a la page table entry que corresponde a va dentro de pdt
//En caso de que todo salga bien, devuelve 1 y en pte el puntero a la pte correspondiente
//Si la page table no existe, create_page_table determina si se crea una nueva, o se devuelve error
//Posibles valores de retorno: -> E_SUCCESS: si todo salio bien
//                             -> E_PTABLE_NOT_PRESENT: si la tabla de paginas no existe y create_page_table==0
//                             -> E_NO_MEMORY: si la creacion de la tabla de paginas fallo
int8_t page_dirwalk(pde_t *pdt, uint32_t va, pte_t **pte, uint8_t create_page_table);

//Decrementa las referencias del page_frame, y si estas llegan a ser nulas, lo agrega al stack de frames libres
void page_dealloc(page_frame_t *frame);

//Obtiene un page frame y lo mapea en va.
//Valores de retorno:   ->E_SUCCESS: si todo salio bien
//                      ->E_NO_MEMORY: si no había más memoria fisica para realizar la operacion                      
uint8_t page_alloc_at_VA( pde_t* pdt, uint32_t va, uint8_t perm );

#endif
