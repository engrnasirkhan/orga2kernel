#ifndef __HANDLERS__H__
#define __HANDLERS__H__

#include <asm/types.h>

extern void init_isrs();
extern void init_irqs();
extern void init_syscalls();
extern void isr_dispatch_routine( struct registers regs );
extern void irq_dispatch_routine( struct registers regs );
extern void set_irq_handler( int irq, handler_t handler );
extern void set_isr_handler( int isr, handler_t handler );
extern void clear_irq_handler( int irq );
extern void clear_isr_handler( int isr );

#endif // __HANDLERS__H_
