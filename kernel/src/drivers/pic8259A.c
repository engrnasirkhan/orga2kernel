#include <asm/types.h>
#include <asm/asm.h>

// Puertos
#define PIC1	0x20
#define PIC2	0xA0
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

// Comandos
#define PIC_EOI	0x20

#define ICW1_ICW4       0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE     0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08		/* Level triggered (edge) mode */
#define ICW1_INIT       0x10		/* Initialization - required! */

#define ICW4_8086        0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO        0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE   0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER  0x0C		/* Buffered mode/master */
#define ICW4_SFNM        0x10		/* Special fully nested (not) */

void pic8259A_remap( int off1, int off2 ) {
	uint8_t m1, m2;

	m1 = inb( PIC1_DATA );
	m2 = inb( PIC2_DATA );

	outb( PIC1_COMMAND, ICW1_INIT+ICW1_ICW4 );  // starts the initialization sequence
	io_wait();
	outb( PIC2_COMMAND, ICW1_INIT+ICW1_ICW4 );
	io_wait();
	outb( PIC1_DATA, off1 );                 // define the PIC vectors
	io_wait();
	outb( PIC2_DATA, off2 );
	io_wait();
	outb( PIC1_DATA, 4 );                       // continue initialization sequence
	io_wait();
	outb( PIC2_DATA, 2 );
	io_wait();
							
	outb( PIC1_DATA, ICW4_8086 );
	io_wait();
	outb( PIC2_DATA, ICW4_8086 );
	io_wait();
																
	outb( PIC1_DATA, m1 );   // restore saved masks.
	outb( PIC2_DATA, m2 );
}

void pic8259A_send_EOI( int irq ) {
	if ( irq >= 8 )
		outb( PIC2_COMMAND, PIC_EOI );
	outb( PIC1_COMMAND, PIC_EOI );
}

void pic8259A_mask( int irq ) {
	uint8_t mascara;
	if ( irq >= 8 ) {
		mascara = inb( PIC2_DATA );
		mascara |= 1 << (irq - 8);
		outb( PIC2_DATA, mascara );
	} else {
		mascara = inb( PIC1_DATA );
		mascara |= 1 << irq;
		outb( PIC2_DATA, mascara );
	}
}

void pic8259A_unmask( int irq ) {
	uint8_t mascara;
	if ( irq >= 8 ) {
		mascara = inb( PIC2_DATA );
		mascara &= ~(1 << (irq - 8));
		outb( PIC2_DATA, mascara );
	} else {
		mascara = inb( PIC1_DATA );
		mascara &= ~(1 << irq);
		outb( PIC2_DATA, mascara );
	}

}

void pic8259A_set_mask( uint32_t mask ) {
	uint8_t mascaraLow = mask & 0xFF;
	uint8_t mascaraHigh = (mask >> 8) & 0xFF;
	outb( PIC1_DATA, mascaraLow );
	outb( PIC2_DATA, mascaraHigh );
}

uint32_t pic8259A_get_mask() {
	uint32_t mascaraLow = inb( PIC1_DATA );
	uint32_t mascaraHigh = inb( PIC2_DATA );
	return mascaraLow | (mascaraHigh << 8);
}

void pic8259A_mask_all() {
	outb( PIC1_DATA, 0xFF );
	outb( PIC2_DATA, 0xFF );
}

void pic8259A_unmask_all() {
	outb( PIC1_DATA, 0x00 );
	outb( PIC2_DATA, 0x00 );
}

void pic8259A_init() {
	pic8259A_remap( 32, 32 + 8 );
	pic8259A_unmask_all();
}
