OUTPUT_FORMAT(binary)
ENTRY(__start)

SECTIONS {
	virtual_start  = 0x40000000; /* 1 GB, espacio de usuario :-) */
	. = virtual_start;

	.text : {
		__text_start = .;
		.obj/src/start.o(.text .rodata .rdata) /* El archivo de inicialización va primero :-) */
		*(.text .rodata .rdata)
	}
	
	.data ALIGN(0x1000) : {
			__data_start = .;
	      *(.data)
	}

	.bss : {
		__bss_start = .;
		*(.bss)
		*(COMMON)
	}
	__bss_end  = .;
}
