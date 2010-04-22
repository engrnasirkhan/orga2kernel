#include <asm/types.h>
#include <asm/syscalls.h>

handler_t syscall_table[] = {
	sys_exit, /* 0 */
	sys_write, /* 1 */
	sys_getpid /* 2 */
};
