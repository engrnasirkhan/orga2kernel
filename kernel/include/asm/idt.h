#ifndef __IDT__H__
#define __IDT__H__

struct IDTEntry {
	unsigned int offset_low : 16;
	unsigned int segment : 16;
	unsigned int cero : 8;
	unsigned int type : 5;
	unsigned int dpl : 2;
	unsigned int present : 1;
	unsigned int offset_high : 16;
};

/**
 * @brief Establece un Interrupt Gate.
 * @param idt La entrada IDT a establecer.
 * @param segment El descriptor de segmento a utilizar.
 * @param offset El offset a utilizar.
 * @param dpl El DPL a utilizar.
 */
void idt_set_interrupt( struct IDTEntry *idt, unsigned int segment, unsigned int offset, unsigned int dpl );

/**
 * @brief Establece un Task Gate.
 * @param idt La entrada IDT a establecer.
 * @param segment El descriptor de segmento a utilizar.
 * @param offset El offset a utilizar.
 * @param dpl El DPL a utilizar.
 */
void idt_set_task( struct IDTEntry *idt, unsigned short tss_selector, unsigned int dpl );

/**
 * @brief Establece un Trap Gate.
 * @param idt La entrada IDT a establecer.
 * @param segment El descriptor de segmento a utilizar.
 * @param offset El offset a utilizar.
 * @param dpl El DPL a utilizar.
 */
void idt_set_trap( struct IDTEntry *idt, unsigned int segment, unsigned int offset, unsigned int dpl );

#endif // __IDT__H__
