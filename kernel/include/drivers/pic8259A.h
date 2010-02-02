#ifndef __PIC8259A__H__
#define __PIC8259A__H__

void pic8259A_remap( int off1, int off2 );
void pic8259A_send_EOI( int irq );
void pic8259A_mask( int irq );
void pic8259A_unmask( int irq );
void pic8259A_set_mask( uint32_t mask );
uint32_t pic8259A_get_mask();
void pic8259A_mask_all();
void pic8259A_unmask_all();
void pic8259A_init();

#endif // __PIC8259A__H__
