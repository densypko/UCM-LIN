#include <linux/module.h> 
#include <asm-generic/errno.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>

#define ALL_LEDS_ON 0x7 //0x7=7
#define ALL_LEDS_OFF 0
#define BUFFER_LENGH 25

struct tty_driver* kbd_driver= NULL;  //driver de modificacion de leds
static struct proc_dir_entry *proc_entry; //entrada al sistema de ficheros de leds



/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void){
   printk(KERN_INFO "modleds: loading\n");
   printk(KERN_INFO "modleds: fgconsole is %x\n", fg_console);
   return vc_cons[fg_console].d->port.tty->driver;
}

/* Set led state to that specified by mask */
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
static ssize_t ledsctl_write(struct file *flip, const char __user *buf, size_t len, loff_t *off) {
	int available_space = BUFFER_LENGH-1;
	int num;
	char kbuf[BUFFER_LENGH];

	if((*off) > 0)
		return 0;
	
	if (len > available_space) {
		printk(KERN_INFO "modleds: ledctl need more space\n");
		return -ENOSPC; //No queda espacio en el dispositivo
	}

	if( copy_from_user(kbuf, buf, len))
		return -EFAULT; //Direccion incorrecta
	kbuf[len] = '\0';
	*off += len;

	if(sscanf(kbuf, "%x", &num) == 1) {
		if((num>=0)&&(num<=7)) {
			num = check(num);
			set_leds(kbd_driver,num);
		}
		else
			printk(KERN_INFO "modleds: mask is NOT correct!!\n");
	}
	else
		printk(KERN_INFO "modleds: mask is NOT correct!!\n");

	return len;
}

static const struct file_operations proc_entry_fops = {
	//.read = ledsctl_read,
	.write = ledsctl_write,
};

static int __init modleds_init(void)
{	
   int ret = 0;
   proc_entry = proc_create("ledctl", 0666, NULL, &proc_entry_fops);
   if(proc_entry == NULL) {
      ret = -ENOMEM;
      printk(KERN_INFO "modleds: ledctl not loaded!!\n");
   }
   else
      printk(KERN_INFO "modleds: ledctl loaded correctly\n");
   kbd_driver= get_kbd_driver_handler();
   set_leds(kbd_driver,ALL_LEDS_ON); 
   return ret;
}

static void __exit modleds_exit(void){
    set_leds(kbd_driver,ALL_LEDS_OFF); 
    remove_proc_entry("ledctl", NULL);
    printk(KERN_INFO "modleds: module unloaded\n");
}




module_init(modleds_init);
module_exit(modleds_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Modleds");
MODULE_AUTHOR("Marco Cuevas, Denys Sypko");
