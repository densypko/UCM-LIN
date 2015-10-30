#include <linux/errno.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

#ifdef __i386__
#define __NR_LED 353
#else
#define __NR_LED 316
#endif

long ledctl(int num) {
	if((long)syscall(__NR_LED, num)!=0)
		return -1;
	return 0;	
}

int main(int argc, void **argv) {
	int num = 0x4;
	return ledctl(num);
}
