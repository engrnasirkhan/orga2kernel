#include <asm/gdt.h>

struct GDTEntry g_GDT[16] __attribute__ ((aligned (16)));

void gdt_fill_code_segment( struct GDTEntry *gdt, void *base, unsigned long limit, unsigned char dpl ) {
	gdt_set_base( gdt, (unsigned long) base );
	gdt_set_limit( gdt, limit );
	gdt->type = 0xA;
	gdt->system = gdt->present = 1;
	gdt->dpl = dpl & 0x3;
	gdt->l = 0;
	gdt->g = gdt->d = 1;
	gdt->avl = 0;
}

void gdt_fill_data_segment( struct GDTEntry *gdt, void *base, unsigned long limit, unsigned char dpl ) {
	gdt_set_base( gdt, (unsigned long) base );
	gdt_set_limit( gdt, limit );
	gdt->type = 0x2;
	gdt->system = gdt->present = 1;
	gdt->dpl = dpl & 0x3;
	gdt->l = 0;
	gdt->g = gdt->d = 1;
	gdt->avl = 0;
}

void gdt_set_base( struct GDTEntry *gdt, unsigned long addr ) {
	gdt->base_low = addr & 0xFFFF;
	gdt->base_mid = (addr >> 16) & 0xFF;
	gdt->base_high = (addr >> 24) & 0xFF;
}

void gdt_set_limit( struct GDTEntry *gdt, unsigned long limit ) {
	gdt->limit_low = limit & 0xFFFF;
	gdt->limit_high = (limit >> 16) & 0xF;
}

unsigned long gdt_get_base( struct GDTEntry *gdt ) {
	return gdt->base_low | (gdt->base_mid << 16) | (gdt->base_high << 24);
}

unsigned long gdt_get_limit( struct GDTEntry *gdt ) {
	return gdt->limit_low | (gdt->limit_high << 16);
}
