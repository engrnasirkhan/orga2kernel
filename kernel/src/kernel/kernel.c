#include <asm/asm.h>
#include <asm/types.h>
#include <boot/multiboot.h>
#include <boot/programs.h>
#include <screen/screen.h>
#include <asm/gdt.h>
#include <asm/asm.h>
#include <asm/types.h>

static reg_t mapa_programa[1024] __attribute__ ((aligned (4096)));

static void ejecutar ( unsigned long phys_start, unsigned long phys_end, char *cmdline ) {
	programs_t *p = (programs_t *) phys_start;
	unsigned int *cr3;
	reg_t entrada_vieja;
	int ret_status;

	if ( p->magic[0] != 'E' ||
		p->magic[1] != 'X' ||
		p->magic[2] != 'E' ||
		p->magic[3] != 0 ) {
		kprint( "Modulo no reconocido :-(\n" );
		return;
	}

	// Llenamos de cero el mapa del programa.
	for ( int i = 0; i < 1024; i++ )
		mapa_programa[i] = 0;
	
	// Páginas de código.
	int paginas = ((unsigned int) p->va_data - (unsigned int) p->va_text) >> 12;
	int i;
	for ( i = 0; i < paginas; i++ )
		mapa_programa[i] = (phys_start + 4096 * i) | 5; // User, Read-only, present
	
	// Páginas de datos.
	paginas = (unsigned int) p->va_bssend;
	paginas = (paginas + 4095) & ~4095; // Redondeamos al próximo múltiplo de página (4KB).
	paginas -= (unsigned int) p->va_data;
	paginas >>= 12;
	paginas += i;
	for ( ; i < paginas; i++ )
		mapa_programa[i] = (phys_start + 4096 * i) | 7; // User, Read-Write, present

	// 1. Obtenemos cr3
	// 2. Establecemos la dirección física de la entrada adecuada.
	// 3. Invalidamos la página
	// 4. Ponemos en cero la sección bss.
	// 5. Saltamos a la función adecuada
	__asm__ __volatile__ ( "movl %%cr3, %0" : "=r"(cr3) );
	unsigned int *entrada = cr3 + (((unsigned int) p->va_text) >> 22);
	entrada_vieja = *entrada;
	*entrada = (((unsigned long) mapa_programa) - 0x80000000) | 3; // System, Read-Write, Present (puntero a tabla de páginas)

	// Podemos o invalidar la página o recargar todo cr3 :P
	//__asm__ __volatile__ ( "invlpg %0" : : "m"(*((unsigned int*)p->va_text)) );
	__asm__ __volatile__ ( "movl %0, %%cr3" : : "r"(cr3) );

	// Ponemos en cero bss (ahora que podemos acceder vía las direcciones virtuales :-)
	unsigned char *bss = p->va_bss;
	while ( bss != p->va_bssend )
		*bss++ = 0;

	__asm__ __volatile__ (
		"pushl %1\n\t"
		"call *%2\n\t"
		"addl $4, %%esp\n\t"
		"movl %%eax, %0"
		: "=rm"(ret_status) : "rm"(cmdline), "r"(p->va_entry)
	);

	// Volvamos a como estábamos
	*entrada = entrada_vieja;
	__asm__ __volatile__ ( "movl %0, %%cr3" : : "r"(cr3) );

	kprint ( "El programa devolvio el codigo de salida: %d\n", ret_status );
}



void kmain(multiboot_info_t*) __noreturn;
void kmain(multiboot_info_t* mbd)
{
    //if flags' bit 0 is set, then mem_lower & mem_upper available
    if(mbd->flags & 0x0001)
    { 
        kprint("Lower memory: %u Kb\n", mbd->mem_lower);
        kprint("Upper memory: %u Kb\n", mbd->mem_upper);
    }
    
    //if flags' bit 6 is set, then memory map is available
    if(mbd->flags & 0x0020)
    {
       memory_map_t *mmap;
 
       kprint("Memory map:\n");
       for(mmap = (memory_map_t *) mbd->mmap_addr;
            (unsigned long) mmap < mbd->mmap_addr + mbd->mmap_length;
            mmap = (memory_map_t *) ((unsigned long) mmap
                                     + mmap->size + sizeof (mmap->size)))
         kprint("  Size = 0x%x, Base_Address = 0x%x,"
                 " Length = 0x%x, Type = 0x%x\n",
                 mmap->size,
                 mmap->base_addr_low,
                 mmap->length_low,
                 mmap->type);                   
    }

	 // Ejecutemos módulo por módulo.
	 if ( mbd->flags & 8 ) {
	 	module_t *mod;
		unsigned long i;

		for ( i = 0, mod = (module_t *) mbd->mods_addr;
				i < mbd->mods_count;
				i++, mod++ ) {
			ejecutar( mod->mod_start, mod->mod_end, (char *) mod->string );
		}
	 }
	
		
	//Tamaño inical de GDT
	unsigned int gdt_tam = 5; 					//contando el nulo 
	struct GDTEntry *direccion_gdt =0x00104010; // arranca ahi
			
	//Creamos 5 entradas a la GDT, del tipo TSS Descriptor
	struct GDTEntry t1;
	struct GDTEntry t2;
	struct GDTEntry t3;
	
	//Seteamos los parametros de los descriptores (base, limite, dpl)
	gdt_fill_tss_segment(&t1,0x0,0x100,0x00); 
	gdt_fill_tss_segment(&t2,0x10,0x100,0x00); 
	gdt_fill_tss_segment(&t3,0x100,0x100,0x01); 
	

	//Agregamos los descriptores a la GDT
	gdt_add_descriptor(direccion_gdt,&gdt_tam,&t1);
	gdt_add_descriptor(direccion_gdt,&gdt_tam,&t2);
	gdt_add_descriptor(direccion_gdt,&gdt_tam,&t3);
	
	//Creamos registro para cargar la GDT
	gdtr_t carga;
	carga.limit= gdt_tam* 8;
	carga.base = *direccion_gdt;   
	
	//Cargamos la nueva GDT
	lgdt(&carga);
	
		 
    while (1);
}
