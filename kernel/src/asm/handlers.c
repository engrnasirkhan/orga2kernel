#include <asm/types.h>
#include <asm/asm.h>
#include <asm/idt.h>
#include <asm/irqs.h>
#include <asm/isrs.h>
#include <asm/syscalls.h>
#include <kernel/globals.h>
#include <kernel/panic.h>
#include <drivers/pic8259A.h>

handler_t g_ISRS[32];
handler_t g_IRQS[16];

void init_isrs() {
	idt_set_trap( g_IDT + 0, 8, (uint32_t) isr_0, 0 );
	idt_set_trap( g_IDT + 1, 8, (uint32_t) isr_1, 0 );
	idt_set_interrupt( g_IDT + 2, 8, (uint32_t) isr_2, 0 );
	idt_set_trap( g_IDT + 3, 8, (uint32_t) isr_3, 0 );
	idt_set_trap( g_IDT + 4, 8, (uint32_t) isr_4, 0 );
	idt_set_trap( g_IDT + 5, 8, (uint32_t) isr_5, 0 );
	idt_set_trap( g_IDT + 6, 8, (uint32_t) isr_6, 0 );
	idt_set_trap( g_IDT + 7, 8, (uint32_t) isr_7, 0 );
	idt_set_trap( g_IDT + 8, 8, (uint32_t) isr_8, 0 ); // TSS (task gate)
	idt_set_trap( g_IDT + 9, 8, (uint32_t) isr_9, 0 );
	idt_set_trap( g_IDT + 10, 8, (uint32_t) isr_10, 0 );
	idt_set_trap( g_IDT + 11, 8, (uint32_t) isr_11, 0 );
	idt_set_trap( g_IDT + 12, 8, (uint32_t) isr_12, 0 );
	idt_set_trap( g_IDT + 13, 8, (uint32_t) isr_13, 0 );
	idt_set_interrupt( g_IDT + 14, 8, (uint32_t) isr_14, 0 ); // Mejor deshabilitar interrupcones en page fault.
	idt_set_trap( g_IDT + 15, 8, (uint32_t) isr_15, 0 );
	idt_set_trap( g_IDT + 16, 8, (uint32_t) isr_16, 0 );
	idt_set_trap( g_IDT + 17, 8, (uint32_t) isr_17, 0 );
	idt_set_trap( g_IDT + 18, 8, (uint32_t) isr_18, 0 );
	idt_set_trap( g_IDT + 19, 8, (uint32_t) isr_19, 0 );
	idt_set_trap( g_IDT + 20, 8, (uint32_t) isr_20, 0 );
	idt_set_trap( g_IDT + 21, 8, (uint32_t) isr_21, 0 );
	idt_set_trap( g_IDT + 22, 8, (uint32_t) isr_22, 0 );
	idt_set_trap( g_IDT + 23, 8, (uint32_t) isr_23, 0 );
	idt_set_trap( g_IDT + 24, 8, (uint32_t) isr_24, 0 );
	idt_set_trap( g_IDT + 25, 8, (uint32_t) isr_25, 0 );
	idt_set_trap( g_IDT + 26, 8, (uint32_t) isr_26, 0 );
	idt_set_trap( g_IDT + 27, 8, (uint32_t) isr_27, 0 );
	idt_set_trap( g_IDT + 28, 8, (uint32_t) isr_28, 0 );
	idt_set_trap( g_IDT + 29, 8, (uint32_t) isr_29, 0 );
	idt_set_trap( g_IDT + 30, 8, (uint32_t) isr_30, 0 );
	idt_set_trap( g_IDT + 31, 8, (uint32_t) isr_31, 0 );

	for ( int i = 0; i < 32; i++ )
		g_ISRS[i] = (handler_t) 0;
}

void init_irqs() {
	idt_set_interrupt( g_IDT + 32, 8, (uint32_t) irq_0, 0 );
	idt_set_interrupt( g_IDT + 33, 8, (uint32_t) irq_1, 0 );
	idt_set_interrupt( g_IDT + 34, 8, (uint32_t) irq_2, 0 );
	idt_set_interrupt( g_IDT + 35, 8, (uint32_t) irq_3, 0 );
	idt_set_interrupt( g_IDT + 36, 8, (uint32_t) irq_4, 0 );
	idt_set_interrupt( g_IDT + 37, 8, (uint32_t) irq_5, 0 );
	idt_set_interrupt( g_IDT + 38, 8, (uint32_t) irq_6, 0 );
	idt_set_interrupt( g_IDT + 39, 8, (uint32_t) irq_7, 0 );
	idt_set_interrupt( g_IDT + 40, 8, (uint32_t) irq_8, 0 );
	idt_set_interrupt( g_IDT + 41, 8, (uint32_t) irq_9, 0 );
	idt_set_interrupt( g_IDT + 42, 8, (uint32_t) irq_10, 0 );
	idt_set_interrupt( g_IDT + 43, 8, (uint32_t) irq_11, 0 );
	idt_set_interrupt( g_IDT + 44, 8, (uint32_t) irq_12, 0 );
	idt_set_interrupt( g_IDT + 45, 8, (uint32_t) irq_13, 0 );
	idt_set_interrupt( g_IDT + 46, 8, (uint32_t) irq_14, 0 );
	idt_set_interrupt( g_IDT + 47, 8, (uint32_t) irq_15, 0 );

	for ( int i = 0; i < 16; i++ )
		g_IRQS[i] = (handler_t) 0;
}

void init_syscalls() {
	idt_set_trap( g_IDT + 0x80, 8, (uint32_t) syscall_entry, 3 );
}


void isr_dispatch_routine( struct registers regs ) {
	static const char *isr_name[32] = {
		"Divide Error (#DE)",
		"Reserved (#DB)",
		"NMI Interrupt (2)",
		"Breakpoint (#BP)",
		"Overflow (#OF)",
		"BOUND Range Exceeded (#BR)",
		"Invalid Opcode (#UD)",
		"Device Not Available (#NM)",
		"Doble Fault (#DF)",
		"Coprocessor Segment Overrun (9)",
		"Invalid TSS (#TSS)",
		"Segment Not Present (#NP)",
		"Stack-Segment Fault (#SS)",
		"General Protection (#GP)",
		"Page Fault (#PF)",
		"Intel Reserved (15)",
		"Math Fault (#MF)",
		"Alignment Check (#AC)",
		"Machine Check (#MC)",
		"SIMD Floating-Point Exception (#XM)"
	};
	if ( !g_ISRS[ regs.nro ] ) {
		if ( regs.nro > 19 )
			panic( "Intel Reserved Exception %d", regs.nro );
		panic( isr_name[ regs.nro ] );
	}

	(*g_ISRS[ regs.nro ])( &regs ); 
}

void irq_dispatch_routine( struct registers regs ) {
	if ( g_IRQS[ regs.nro ] ) {
		(*g_IRQS[ regs.nro ]) (&regs);
	} else {
		debug( "IRQ %d no manejada.", regs.nro );
	}
	pic8259A_send_EOI( regs.nro );
}

void set_irq_handler( int irq, handler_t handler ) {
	if ( irq < 0 || irq > 15 )
		panic( "Tratando de establecer un IRQ inexistente: %d", irq );
	g_IRQS[irq] = handler;
}

void set_isr_handler( int isr, handler_t handler ) {
	if ( isr < 0 || isr > 31 )
		panic( "Tratando de establecer un ISR inexistente: %d", isr );
	g_ISRS[isr] = handler;
}

void clear_irq_handler( int irq ) {
	if ( irq < 0 || irq > 15 )
		panic( "Tratando de quitar un IRQ inexistente: %d", irq );
	g_IRQS[irq] = 0;
}

void clear_isr_handler( int isr ) {
	if ( isr < 0 || isr > 15 )
		panic( "Tratando de quitar un ISR inexistente: %d", isr );
	g_ISRS[isr] = 0;
}

void syscall_table_entry( struct registers regs ) {
	kprint("Syscall %d\n", regs.eax );
	if ( regs.eax > NR_SYSCALLS  || !syscall_table[regs.eax] ) {
		debug( "Syscall %d desconocida.\n", regs.eax );
		return;
	}

	regs.eax = (*syscall_table[regs.eax])(&regs);
}
