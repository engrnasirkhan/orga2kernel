#include <asm/types.h>
#include <asm/syscalls.h>

handler_t syscall_table[] = {
	NULL, /* 0 */
	sys_write /* 1 */
};
