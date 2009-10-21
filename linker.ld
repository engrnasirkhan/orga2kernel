ENTRY(loader)

PHDRS {
	codigo PT_LOAD AT(0x100000);
	datos PT_LOAD;
}

SECTIONS {
	phys = 0x100000;
	virt = 0x100000;
	. = virt;
	
	.text virt : AT(phys) {
	      *(.text .rodata .rdata)
	} :codigo
	
	.data ALIGN(0x1000) : {
	      *(.data)
	} :datos

	.bss : {
	     *(.bss)
		  *(COMMON)
	} :datos
}
