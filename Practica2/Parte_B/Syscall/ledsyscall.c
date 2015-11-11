#include <linux/syscalls.h> /* For SYSCALL_DEFINEi() */
#include <linux/kernel.h>
#include <asm-generic/errno.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>
#include <asm-generic/errno-base.h>
#include <linux/errno.h>


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
int convert(int num) {
	int i;
	int status = 0;	
	for(i=0;i<3;i++){
	  if(num&(0x1 << i)) {
	  	switch(i) {
			case 0: status |=1;
			case 1: status |=4;
			case 2: status |=2;
		}
		printf("Status ==> %d\n",status);
	  }	
	}
	return status;
}

/* This syscall modify the the led state */
SYSCALL_DEFINE1(ledctl,unsigned int,leds) {
	if((leds<0)||(leds>7))
		return -EINVAL;
	kbd_driver= get_kbd_driver_handler();

	if(kbd_driver==NULL)
		return -ENODEV;

	leds = convert(leds);
	if(set_leds(kbd_driver,leds) < 0)
		return -ENOTSUPP;
	return 0;
}

/* Authors:
 *--------- Marco Antonio Cuevas Redondo
 *--------- Denys Sypko
 */
