#include <linux/syscalls.h> /* For SYSCALL_DEFINEi() */
#include <linux/kernel.h>
//#include <linux/module.h> 
#include <asm-generic/errno.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>


struct tty_driver* kbd_driver= NULL;  //driver de modificacion de leds
static struct proc_dir_entry *proc_entry; //entrada al sistema de ficheros de leds

/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void){
   printk(KERN_INFO "modleds: loading\n");
   printk(KERN_INFO "modleds: fgconsole is %x\n", fg_console);
   return vc_cons[fg_console].d->port.tty->driver;
}

static inline int set_leds(struct tty_driver* handler, unsigned int mask){
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
}

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

SYSCALL_DEFINE1(ledctl,unsigned int,leds) {
	if((leds>=0)&&(leds<=7)) {
		num = check(num);
		set_leds(kbd_driver,num);
	}
	
	else
		printk(KERN_INFO "modleds: mask is NOT correct!!\n");
}

