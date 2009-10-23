#ifndef __GDT__H__
#define __GDT__H__

/** @file gdt.h
 *
 *  La clase GDTEntry representa un descriptor de segmento de la GDT. Posee algunas funciones
 *  auxiliares para crear segmentos de código y datos.
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

	/** @brief Establece el tipo del segmento en código, RX. */ 
	void inline setCodeType() { type = 0xA; /* RX */ }

	/** @brief Establece el tipo del segmento en datos, RW. */ 
	void inline setDataType() { type = 0x2; /* RW */ }

	/** @brief Establece la base del segmento.
	 *  @param p Un puntero a la dirección base del segmento.
	 */
	void inline setBase( void *p ) { setBase( (unsigned long)p ); }

	/** @brief Establece la base del segmento.
	 *  @param addr La dirección base del segmento.
	 */
	void inline setBase( unsigned long addr ) {
		base_low = addr & 0xFFFF;
		base_mid = (addr >> 16) & 0xFF;
		base_high = (addr >> 24) & 0xFF;
	}

	/** @brief Establece el límite del segmento.
	 *  @param limit El límite del segmento. La interpretación de
	 *               este parámetro depende del bit de granularidad.
	 */
	void inline setLimit( unsigned long limit ) {
		limit_low = limit & 0xFFFF;
		limit_high = (limit >> 16) & 0xF;
	}

	/** @brief Establece un segmento de código.
	 *  @param base Un puntero a la dirección base del segmento.
	 *  @param limit El límite del segmento.
	 *  @param dpl El nivel de privilegio del segmento.
	 *
	 *  Rellena el segmento con los datos pasados como parámetros.
	 *  El segmento producido es de 32 bits, con granularidad de 4KB,
	 *  está presente y AVL=0.
	 */
	void fillCodeSegment( void *base, unsigned long limit, unsigned char dpl ) {
		setBase( base );
		setLimit( limit );
		setCodeType();
		system = present = 1;
		this->dpl = dpl & 0x3;
		l = 0;
		g = d = 1;
		avl = 0;
	}

	/** @brief Establece un segmento de datos.
	 *  @param base Un puntero a la dirección base del segmento.
	 *  @param limit El límite del segmento.
	 *  @param dpl El nivel de privilegio del segmento.
	 *
	 *  Rellena el segmento con los datos pasados como parámetros.
	 *  El segmento producido es de 32 bits, con granularidad de 4KB,
	 *  está presente y AVL=0.
	 */
	void fillDataSegment( void *base, unsigned long limit, unsigned char dpl ) {
		setBase( base );
		setLimit( limit );
		setDataType();
		system = present = 1;
		this->dpl = dpl & 0x3;
		l = 0;
		g = d = 1;
		avl = 0;
	}

	/** @brief Devuelve la base del segmento. */
	unsigned long inline getBase() const { return base_low | (base_mid << 16) | (base_high << 24); }
	/** @brief Devuelve el límite del segmento. */
	unsigned long inline getLimit() const { return limit_low | (limit_high << 16); }
};

#endif //__GDT__H__
