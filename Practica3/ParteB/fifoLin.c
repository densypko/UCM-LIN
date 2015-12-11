#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include "cbuffer.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("FifoLin Kernel Module - FDI-UCM");
MODULE_AUTHOR("Marco Cuevas, Denys Sypko");

#define BUFFER_LENGTH 100

DEFINE_SEMAPHORE(mtx);
int prod_count, cons_count, prod_waiting, cons_waiting;
struct semaphore prod, cons;
struct proc_dir_entry *proc_entry; // entrada de /proc
cbuffer_t *cbuffer;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// LECTURA, ESCRITURA, APERTURA Y CIERRE /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int open_fifoLin(struct inode *node, struct file *filp) {
	if(down_interruptible(&mtx)) return -EINTR;
	if(filp->f_mode & FMODE_READ) {
		cons_count++;
		if(prod_waiting > 0) {	
			up(&prod);
			prod_waiting--;		
		}

		while(prod_count<=0) {
			cons_waiting++;
			up(&mtx);
			if(down_interruptible(&cons)) {
				down(&mtx);
				cons_waiting--;
				up(&mtx);
				return -EINTR;
			}
			if(down_interruptible(&mtx)) return -EINTR;
		}		
	}
	else {
		prod_count++;
		if(cons_waiting > 0) {
			up(&cons);
			cons_waiting--;		
		}

		while(cons_count<=0) {
			prod_waiting++;
			up(&mtx);
			if(down_interruptible(&prod)) {
				down(&mtx);
				prod_waiting--;
				up(&mtx);
				return -EINTR;
			}
			if(down_interruptible(&mtx)) return -EINTR;
		}		
	}
	up(&mtx);
	printk(KERN_INFO "FifoLin: Productores ==> %d  Consumidores ==> %d\n",prod_count, cons_count);
	printk(KERN_INFO "FifoLin: ProdWaiting ==> %d  ConsuWaiting ==> %d\n",prod_waiting, cons_waiting);
	printk(KERN_INFO "FifoLin: /proc/fifoLin opened\n");
	return 0;
}

static int release_fifoLin(struct inode *node, struct file *filp) {
	if(down_interruptible(&mtx)) return -EINTR;
	if(filp->f_mode & FMODE_READ) {
		cons_count--;
		if(prod_waiting > 0) {
			up(&prod);
			prod_waiting--;		
		}
	}
	else {
		prod_count--;
		if(cons_waiting > 0) {
			up(&cons);
			cons_waiting--;		
		}
	}
	if((cons_count==0)&&(prod_count==0))
		clear_cbuffer_t(cbuffer);
	up(&mtx);
	printk(KERN_INFO "FifoLin: Productores ==> %d  Consumidores ==> %d\n",prod_count, cons_count);
	printk(KERN_INFO "FifoLin: ProdWaiting ==> %d  ConsuWaiting ==> %d\n",prod_waiting, cons_waiting);
	printk(KERN_INFO "FifoLin: /proc/fifoLin closed\n");
	return 0;
}

static ssize_t write_fifoLin(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
	char kbuf[BUFFER_LENGTH+1];

	if ((*off) > 0)
    		return 0;

	if(len>BUFFER_LENGTH)
		return -ENOMEM;
	copy_from_user(kbuf, buf, len);
	kbuf[len] = '\0';
	if(down_interruptible(&mtx)) return -EINTR;
		while((nr_gaps_cbuffer_t(cbuffer)<len) && (cons_count > 0)) {
			prod_waiting++;
			up(&mtx);
			if(down_interruptible(&prod)) {
				down(&mtx);
				prod_waiting--;
				up(&mtx);
				return -EINTR;
			}
			if(down_interruptible(&mtx)) return -EINTR;
		}
		if(cons_count <= 0) return -EPIPE;
		insert_items_cbuffer_t(cbuffer, kbuf, len);
		
		if(cons_waiting > 0) {
			up(&cons);
			cons_waiting--;
		}
	up(&mtx);
	printk(KERN_INFO "FifoLin: Productores ==> %d  Consumidores ==> %d\n",prod_count, cons_count);
	printk(KERN_INFO "FifoLin: ProdWaiting ==> %d  ConsuWaiting ==> %d\n",prod_waiting, cons_waiting);
	printk(KERN_INFO "FifoLin: /proc/fifoLin writed\n");
	return len;
}

static ssize_t read_fifoLin(struct file *filp, char __user *buf, size_t len, loff_t *off) {
	char kbuf[BUFFER_LENGTH+1];
	
	if ((*off) > 0)
    		return 0;
	if (len > BUFFER_LENGTH)
		return -ENOMEM;

	if(down_interruptible(&mtx)) return -EINTR;
		while((len > size_cbuffer_t(cbuffer)) && (prod_count > 0)) {
			cons_waiting++;
			up(&mtx);
			if(down_interruptible(&cons)) {
				down(&mtx);
				cons_waiting--;
				up(&mtx);
				return -EINTR; 
			}
			if(down_interruptible(&mtx)) return -EINTR;
		}
		if(prod_count == 0 && is_empty_cbuffer_t(cbuffer)) {
			up(&mtx);
			return 0;
		}
		remove_items_cbuffer_t(cbuffer, kbuf, len);
		
		if(prod_waiting > 0) {
			up(&prod);
			prod_waiting--;
		}
	up(&mtx);

	copy_to_user(buf, kbuf, len);
	printk(KERN_INFO "FifoLin: Productores ==> %d  Consumidores ==> %d\n",prod_count, cons_count);
	printk(KERN_INFO "FifoLin: ProdWaiting ==> %d  ConsuWaiting ==> %d\n",prod_waiting, cons_waiting);
	printk(KERN_INFO "FifoLin: /proc/fifoLin readed\n");
	return len;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// INICIO Y FIN DEL MODULO ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const struct file_operations fops = {
	.open    = open_fifoLin,
	.read    = read_fifoLin,
	.write   = write_fifoLin,
	.release = release_fifoLin,
};

int init_fifoLin_module( void ) {
	int ret = 0;
	prod_count=0; prod_waiting=0;
	cons_count=0; cons_waiting=0;
	sema_init(&prod, 0);
	sema_init(&cons,0);
	cbuffer = create_cbuffer_t(BUFFER_LENGTH);
	proc_entry = proc_create( "fifoLin", 0666, NULL, &fops); 

	if (proc_entry == NULL) {
		ret = -ENOMEM; // No hay bastante espacio
		printk(KERN_INFO "FifoLin: Can't create /proc/fifoLin entry\n");
	}
	else
		printk(KERN_INFO "FifoLin: Module fifoLin loaded\n");
	//try_module_get(THIS_MODULE);
	return ret;
}


void exit_fifoLin_module( void ) {
	destroy_cbuffer_t ( cbuffer );
	remove_proc_entry("fifoLin", NULL); // eliminar la entrada del /proc
	printk(KERN_INFO "FifoLin: Module fifoLin unloaded.\n");
	//module_put(THIS_MODULE);
}


module_init( init_fifoLin_module );
module_exit( exit_fifoLin_module );
