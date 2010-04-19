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

/**
 * Inicializa la Memory Management Unit. Para ello utilizando la informacion provista 
 * por GRUB inicializa las estructuras de control de memoria y se encarga de llamar 
 * a las funciones necesarias para instalar la GDT definitiva y pasar a paginacion
 *
 * @param mdb Puntero a estructura multiboot_info_t provista por GRUB al inicio del kernel
 * @see mmu_install_gdt
 * @see mmu_init_paging
 * @see mmu_build_kernel_heap
 */
void mmu_init(multiboot_info_t* mbd);

/**
 * Toma un frame fisico libre de 4kb y lo mapea dentro de la primera direccion virtual disponible para la 
 * PDT pasada como parametro con los permisos solicitados. Devuelve ademas la direccion fisica asociada.
 *
 * @param pdt Puntero a la pdt sobre la cual se quiere realizar el mapeo
 * @param va  Puntero donde se guarda la direccion virtual donde se realizo el mapeo
 * @param pa  Puntero donde se guarda la direccion fisica
 * @param perm Permisos asignados a la pagina mapeada
 * @return Devuelve E_MMU_SUCCESS si se pudo realizar el alloc correctamente, o E_MMU_NO_MEMORY en caso de error
 * @see mmu_free
 * @see mmu_alloc_at_VA
 * @see mmu_map_pa2va
 */
uint8_t mmu_alloc(uint32_t *pdt, uint32_t *va, uint32_t *pa, uint8_t perm);
           
/**
 * Obtiene un frame libre y lo intenta mapear en la direccion virtual pasada como parametro.
 *
 * @param pdt Puntero a la PDT sobre la cual se realiza el mapeo
 * @param va Direccion virtual usada para mapear el frame libre
 * @param perm Permisos asignados a la nueva pagina mapeada
 * @param force_dealloc Si en la va pasada como parametro ya habia un mapeo previo, entonces lo deshace si force_dealloc==1
 * @return Devuelve E_MMU_SUCCESS si se realizo el mapeo correctamente, E_MMU_INVALID_VA si ya habia un mapeo y force_dealloc==0 o E_MMU_NO_MEMORY si no se pudo realizar
 * @see mmu_alloc
 * @see mmu_map_pa2va
 * @see mmu_free
 */
uint8_t mmu_alloc_at_VA( pde_t *pdt, uint32_t va, uint8_t perm, uint8_t force_dealloc );

/**
 * Intenta asociar la va con la pa pasada como parametro para la pdt solicitada.
 *
 * @param pdt Puntero a la PDT sobre la cual se realiza el mapeo
 * @param va Direccion fisica usada para realizar el mapeo
 * @param va Direccion virtual usada para mapear la direccion fisica
 * @param perm Permisos asignados a la nueva pagina mapeada
 * @param force_dealloc Si en la va pasada como parametro ya habia un mapeo previo, entonces lo deshace si force_dealloc==1
 * @return Devuelve E_MMU_SUCCESS si se realizo el mapeo correctamente, E_MMU_INVALID_VA si ya habia un mapeo y force_dealloc==0 o E_MMU_NO_MEMORY si no se pudo realizar
 * @see mmu_alloc
 * @see mmu_alloc_at_VA
 * @see mmu_free
 */
uint8_t mmu_map_pa2va(pde_t *pdt, uint32_t pa, uint32_t va, uint8_t perm, uint8_t force_dealloc );

/**
 * Dada una PDT, deshace el mapeo de la va, obtiene el frame correspondiente, decrementa su cantidad de 
 * referencias (si son 0 lo apila como libre) y realiza invlpg para limpiar la TLB
 *
 * @param pdt Puntero a una PDT
 * @param va  Direccion virtual a liberar. Debe ser multiplo de PAGE_SIZE
 * @return uint8_t Devuelve E_MMU_SUCCESS si la operacion se realizo con exito, o E_MMU_INVALID_VA si la va no pudo ser desmapeada
 * @see mmu_alloc
 * @see mmu_alloc_at_VA
 * @see invlpg
 */
uint8_t mmu_free( pde_t *pdt, uint32_t va );

/**
 * Devuelve la cantidad de frames libres en memoria fisica
 *
 * @return uint32_t
 */
uint32_t mmu_get_free_frame_count(); 

/**
 * Instala una nueva PDT para ser usada por una nueva tarea
 *
 * @param va Es un puntero a un uint32_t que se va a utilizar para guardar la va utilizada para mapear la nueva pdt dentro del contexto del kernel
 * @param pa Es un puntero a un uint32_t que se va a utililar para guardar la pa utilizada para guardar la nueva pdt
 * @return uint8_t Devuelve E_MMU_SUCCESS si se realizo con exito la instalacion, o E_MMU_NO_MEMORY si no hubo memoria disponible para realizar la operacion  
 * @see mmu_uninstall_task_pdt
 */
uint8_t mmu_install_task_pdt(uint32_t *va, uint32_t *pa);

/**
 * Desinstala la PDT de una tarea pasada como parametro, realizando todos los desmapeos correspondientes
 * 
 * @param task_pdt Puntero a la PDT de la taerea
 * @return uint8_t
 * @see mmu_install_task_pdt 
 */
uint8_t mmu_uninstall_task_pdt(pde_t *task_pdt);

/**
 * Toma un frame fisico libre de 4kb y devuelve su dirección virtual en el contexto del kernel.
 *
 * @param va  Puntero donde se guarda la dirección virtual.
 * @return Devuelve E_MMU_SUCCESS si se pudo realizar el alloc correctamente, o E_MMU_NO_MEMORY en caso de error
 * @see mmu_free
 * @see mmu_alloc_at_VA
 * @see mmu_map_pa2va
 */
uint8_t mmu_kalloc( uint32_t *va );

#endif //__MMU__H__
