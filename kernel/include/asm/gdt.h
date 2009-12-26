#ifndef __GDT__H__
#define __GDT__H__

/** @file gdt.h
 *  @brief Funciones para el manejo de descriptores de la GDT.
 */

/** @brief Una entrada de la GDT
 */
struct GDTEntry {
	unsigned int limit_low : 16;
	unsigned int base_low : 16;
	unsigned int base_mid : 8;
	unsigned int type : 4;
	unsigned int system : 1; /* System: 0 = system, 1 = code or data */
	unsigned int dpl : 2; /* DPL */
	unsigned int present : 1; /* Present */
	unsigned int limit_high : 4;
	unsigned int avl : 1; /* Available */
	unsigned int l : 1; /* Long: 0 = 32 bits, 1= 64 bits */
	unsigned int d : 1; /* Operation size: 0 = 16 bits, 1 = 32 bits */
	unsigned int g : 1; /* Granularity: 0 = Byte, 1 = 4KB */
	unsigned int base_high : 8;
};

/** @brief Establece la base del segmento.
 *  @param addr La dirección base del segmento.
 */
void gdt_set_base( struct GDTEntry *gdt, unsigned long addr );

/** @brief Establece el límite del segmento.
 *  @param limit El límite del segmento. La interpretación de
 *               este parámetro depende del bit de granularidad.
 */
void gdt_set_limit( struct GDTEntry *gdt, unsigned long limit );

/** @brief Devuelve la base del segmento. */
unsigned long gdt_get_base( struct GDTEntry *gdt );

/** @brief Devuelve el límite del segmento. */
unsigned long gdt_get_limit( struct GDTEntry *gdt );

/** @brief Establece un segmento de código.
 *  @param gdt La entrada a establecer.
 *  @param base Un puntero a la dirección base del segmento.
 *  @param limit El límite del segmento.
 *  @param dpl El nivel de privilegio del segmento.
 *
 *  Rellena el segmento con los datos pasados como parámetros.
 *  El segmento producido es de 32 bits, con granularidad de 4KB,
 *  está presente y AVL=0.
 */
void gdt_fill_code_segment( struct GDTEntry *gdt, void *base, unsigned long limit, unsigned char dpl );

/** @brief Establece un segmento de datos.
 *  @param gdt La entrada a establecer.
 *  @param base Un puntero a la dirección base del segmento.
 *  @param limit El límite del segmento.
 *  @param dpl El nivel de privilegio del segmento.
 *
 *  Rellena el segmento con los datos pasados como parámetros.
 *  El segmento producido es de 32 bits, con granularidad de 4KB,
 *  está presente y AVL=0.
 */
void gdt_fill_data_segment( struct GDTEntry *gdt, void *base, unsigned long limit, unsigned char dpl );



/** @brief Establece un segmento de tss.
 *  @param gdt La entrada a establecer.
 *  @param base Un puntero a la dirección base del segmento.
 *  @param limit El límite del segmento.
 *  @param dpl El nivel de privilegio del segmento.
 *
 *  Rellena el segmento con los datos pasados como parámetros.
 *  El segmento producido es de 32 bits, con granularidad de 4KB,
 *  está presente y AVL=0.
 */
void gdt_fill_tss_segment( struct GDTEntry *gdt, void *base, unsigned long limit, unsigned char dpl );


void gdt_add_descriptor(struct GDTEntry *gdt_address,unsigned int *cant,struct GDTEntry *gdt);

void gdt_print(struct GDTEntry *gdt_address,unsigned int cant);
#endif //__GDT__H__
