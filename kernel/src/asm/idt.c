#include <asm/types.h>
#include <asm/idt.h>

struct IDTEntry g_IDT[256] __attribute__ ((aligned (16)));
idtr_t g_IDTr __attribute__ ((aligned (16)));

void idt_set_interrupt( struct IDTEntry *idt, unsigned int segment, unsigned int offset, unsigned int dpl )  {
	idt->offset_low = offset & 0xFFFF;
	idt->segment = segment;
	idt->cero = 0;
	idt->type = 0xE; // 32 bits.
	idt->dpl = dpl;
	idt->present = 1;
	idt->offset_high = offset >> 16;
}

void idt_set_task( struct IDTEntry *idt, unsigned short tss_selector, unsigned int dpl ) {
	idt_set_interrupt( idt, tss_selector, 0, dpl );
	idt->type = 0x5;
}

void idt_set_trap( struct IDTEntry *idt, unsigned int segment, unsigned int offset, unsigned int dpl ) {
	idt_set_interrupt( idt, segment, offset, dpl );
	idt->type = 0xF;
}
