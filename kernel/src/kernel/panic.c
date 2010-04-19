#include <asm/asm.h>
#include <screen/screen.h>
#include <kernel/panic.h>

void debug( const char *fmt, ... ) {
	while ( *fmt )
		kputc( *fmt++ );
}

// TODO: Hacerlo bien (necesitamos un kprint(const char *, va_arg);
void panic( const char *fmt, ... ) {
	debug(fmt);
	dumpregs();
	stacktrace(2,10);
	cli(); for(;;) hlt();
}

void dumpregs() {
	struct registers regs;

	__asm__ __volatile__ (
		"movl %%eax, %0\n\t"
		"movl %%ebx, %1\n\t"
		"movl %%ecx, %2\n\t"
		"movl %%edx, %3\n\t"
		"movl %%esi, %4\n\t"
		"movl %%edi, %5\n\t"
		"movl %%ebp, %6\n\t"
		"movl %%esp, %7\n\t"
		"pushfl\n\t"
		"popl %8\n\t"
		"call 1f\n\t"
		"1:\n\t"
		"popl %9"
		: "=rm"(regs.eax),
		"=rm"(regs.ebx),
		"=rm"(regs.ecx),
		"=rm"(regs.edx),
		"=rm"(regs.esi),
		"=rm"(regs.edi),
		"=rm"(regs.ebp),
		"=rm"(regs.esp),
		"=rm"(regs.eflags),
		"=rm"(regs.eip)
	);
	dumpregs_regs( &regs );
}

void dumpregs_regs( struct registers *r ) {
	kprint( "EAX: 0x%x EBX: 0x%x ECX: 0x%x EDX: 0x%x\n"
	        "ESI: 0x%x EDI: 0x%x EBP: 0x%x ESP: 0x%x\n"
			  "EFLAGS: 0x%x\n"
			  "EIP: 0x%x\n",
			  r->eax, r->ebx, r->ecx, r->edx,
			  r->esi, r->edi, r->ebp, r->esp,
			  r->eflags,
			  r->eip );
}

void stacktrace( int saltar, int cuantas ) {
	reg_t *ebp;
	__asm__ __volatile__ ( "movl %%esp, %0" : "=rm"(ebp) );

	while ( saltar-- ) ebp = (reg_t *) *ebp;

	while ( cuantas-- && ebp ) {
		kprint( "ebp: 0x%x, eip: 0x%x, args: 0x%x, 0x%x, 0x%x\n",
			*ebp, *(ebp + 4), *(ebp + 8), *(ebp + 12), *(ebp + 16) );
		ebp = (reg_t *) *ebp;
	}
}
