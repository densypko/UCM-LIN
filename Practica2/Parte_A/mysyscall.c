#include <linux/syscalls.h>
#include <linux/kernel.h>

SYSCALL_DEFINE0(lin_hello) {
	printk(KERN_DEBUG "Hello world\n");
	return 0;
}
