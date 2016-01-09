#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/timer.h>

#define BUFFER_LENGTH       100

struct timer_list my_timer; /* Structure that describes the kernel timer */
cbuffer_t *cbuffer;

/* Work descriptor */
struct work_struct my_work;

///////////////////////// Lista Enlazada ////////////////////////////////////////////////////////////////////////
struct list_item{
  int data;
  struct list_head links;
};
static struct list_head my_list; // nodo fantasma

//////////////////////// Parametros Configurables desde /proc/modconfig/////////////////////////////////////////

int timer_period = 250; // temporizador se activa cada timer_period  ticks 
int max_random = 300;  // el nº aleat. maximo.
int emergency_threshold = 80; // el % de ocupación que provoca la activación de la tarea 

static struct proc_dir_entry *proc_entry_modtimer; // entrada de /proc
static struct proc_dir_entry *proc_entry_modconfig; // entrada de /proc



/* Work's handler function */ 
/* La función asociada a la tarea (my_work) volcará los datos del buﬀer a la lista enlazada de enteros */
static void my_wq_function( struct work_struct *work )
{
    printk(KERN_INFO "HELLO WORLD!!\n");
}



/* Function invoked when timer expires (fires) */
static void fire_timer(unsigned long data)
{
	static char flag=0;
        
        if (flag==0)
            printk(KERN_INFO "Tic\n");
        else
            printk(KERN_INFO "Tac\n");           
	
        flag=~flag;
        
        /* Re-activate the timer one second from now */
	mod_timer( &(my_timer), jiffies + HZ); 
}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// MODTIMER: APERTURA,LECTURA Y CIERRE /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int open_modtimer(struct inode *node, struct file *filp) {
	
	return 0;
}

static ssize_t read_modtimer(struct file *filp, char __user *buf, size_t len, loff_t *off) {
	
	return len;
}

static int release_modtimer(struct inode *node, struct file *filp) {

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// MODCONFIG: ESCRITURA,LECTURA ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ssize_t write_modconfig(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

	return len;
}

static ssize_t read_modconfig(struct file *filp, char __user *buf, size_t len, loff_t *off) {
	
	return len;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// INICIO Y FIN DEL MODULO ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const struct file_operations proc_entry_modtimer_fops = {
    .open    = open_modtimer,
	.read    = read_modtimer,
	.release = release_modtimer,
};
static const struct file_operations proc_entry_modconfig_fops = {
	.write   = write_modconfig,
	.read    = read_modconfig,

};



int init_timer_module( void )
{
    int ret=0;

	proc_entry_modtimer = proc_create( "modtimer", 0666, NULL, &proc_entry_modtimer_fops);
	if (proc_entry_modtimer == NULL) {
    	ret = -ENOMEM; // No hay bastante espacio
    	printk(KERN_INFO "Modtimer: Can't create /proc entry\n");
  	}else {
    	printk(KERN_INFO "Modtimer: Module loaded\n");
  	}
	
	proc_entry_modconfig = proc_create( "modconfig", 0666, NULL, &proc_entry_modconfig_fops); 
	if (proc_entry_modconfig == NULL) {
    	ret = -ENOMEM; // No hay bastante espacio
    	printk(KERN_INFO "Modconfig: Can't create /proc entry\n");
  	}else {
    	printk(KERN_INFO "Modconfig: Module loaded\n");
  	}
	
    //////????????????????????????//////////////
	/* Initialize work structure (with function) */
	  	INIT_WORK(&my_work, my_wq_function );

	  	/* Enqueue work */
	  	schedule_work(&my_work);

	
		/* Create timer */
		init_timer(&my_timer);
		/* Initialize field */
		my_timer.data=0;
		my_timer.function=fire_timer;
		my_timer.expires=jiffies + HZ;  /* Activate it one second from now */
		/* Activate the timer for the first time */
		add_timer(&my_timer);
 
    return ret;
}


void cleanup_timer_module( void ){
  /* Wait until completion of the timer function (if it's currently running) and delete timer */
  del_timer_sync(&my_timer);
  
  remove_proc_entry("modtimer", NULL); // eliminar la entrada del /proc
  printk(KERN_INFO "Modtimer: Module unloaded.\n");
  
  remove_proc_entry("modconfig", NULL); // eliminar la entrada del /proc
  printk(KERN_INFO "Modconfig: Module unloaded.\n");
}

module_init( init_timer_module );
module_exit( cleanup_timer_module );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("timermod Module");
MODULE_AUTHOR("Marco Cuevas, Denys Sypko");
