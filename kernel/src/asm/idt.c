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
