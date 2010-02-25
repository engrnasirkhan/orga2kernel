ENTRY(boot)

SECTIONS {
   	physical_start = 0x00100000;
	virtual_start  = 0x80100000;
   
   . = virtual_start;

   .text : AT(ADDR(.text) - 0x80000000) {
       *(.text)
       *(.rodata*)
   }

   .data ALIGN (0x1000) : AT(ADDR(.data) - 0x80000000) {
       *(.data)
   }

   .bss : AT(ADDR(.bss) - 0x80000000) {
       _sbss = .;
       *(COMMON)
       *(.bss)
       _ebss = .;
   }
   
   __kernel_end = .;
}

