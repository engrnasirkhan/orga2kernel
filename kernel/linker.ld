ENTRY(boot)

PHDRS {
	init PT_LOAD FLAGS(7); /* RWX */
	codigo PT_LOAD FLAGS(5); /* R-X */
	datos PT_LOAD FLAGS(6); /* RW- */
}

SECTIONS {
	physical_start = 0x00100000;
	virtual_start  = 0x80100000;
	. = virtual_start;

	/* Las rutinas de inicialización van en estas secciones
	 * para así poder descartarlas luego de la inicialización
	 * y liberar memoria */
	.init.text virtual_start : AT(physical_start) {
		*(.init.text .init.rodata)
	} :init 

	.init.data : {
		*(.init.data)
	} :init

	.init.bss : {
		*(.init.bss)
	} :init

	/* Las secciones comunes */
	/* Definimos __kernel_start y __kernel_end para
	 * saber donde está el inicio y el fin del kernel
	 */
	. = ALIGN(0x1000);
	__kernel_start = .;
	.text : {
		*(.text .rodata .rdata)
	} :codigo
	
	.data ALIGN(0x1000) : {
	      *(.data)
	} :datos

	.bss : {
	     *(.bss)
	     *(COMMON)
	} :datos
	__kernel_end = .;
}
