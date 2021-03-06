#ifndef __GLOBALS__H__
#define __GLOBALS__H__

#include <asm/types.h>
#include <asm/idt.h>
#include <asm/gdt.h>

/** Espacio para 15 entradas GDT + descriptor nulo. */
extern struct GDTEntry g_GDT[24];

/** Espacio para 256 interrupciones */
extern struct IDTEntry g_IDT[256];
extern idtr_t g_IDTr;

/** El Commandline, 256 caracteres por defecto **/
extern char g_cmdline[256];

extern handler_t g_ISRS[32];
extern handler_t g_IRQS[16];

#endif // __GLOBALS__H__
