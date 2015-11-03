#include <linux/syscalls.h> /* For SYSCALL_DEFINEi() */
#include <linux/kernel.h>
#include <asm-generic/errno.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>

struct tty_driver* kbd_driver= NULL;  /* Driver of leds */

/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void){
   printk(KERN_INFO "sys_ledctl: loading\n");
   printk(KERN_INFO "sys_ledctl: fgconsole is %x\n", fg_console);
   return vc_cons[fg_console].d->port.tty->driver;
}

/* Set led state to that specified by mask */
static inline int set_leds(struct tty_driver* handler, unsigned int mask){
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
}

/* Set input num to correctly mask */
int check(int num) {
	int ret = num;
	switch(num) {
		case 2: ret = 4; break;
		case 3: ret = 5; break;
		case 4: ret = 2; break;
		case 5: ret = 3; break;
	}
	return ret;
}

/* This syscall modify the the led state */
SYSCALL_DEFINE1(ledctl,unsigned int,leds) {
	if((leds<0)||(leds>7))
		return -1;
	kbd_driver= get_kbd_driver_handler();
	leds = check(leds);
	set_leds(kbd_driver,leds);
	return 0;
}

/* Authors:
 *--------- Marco Antonio Cuevas Redondo
 *--------- Denys Sypko
 */
