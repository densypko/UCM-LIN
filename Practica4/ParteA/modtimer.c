#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <linux/random.h>
#include <asm-generic/uaccess.h>
#include <asm-generic/errno.h>
#include <linux/spinlock_types.h>
#include <linux/semaphore.h>
#include <linux/smp.h>
#include "cbuffer.h"

#define BUFFER_LENGTH       100
#define CBUF_LENGTH			10

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
int max_random = 300;  // el nº aleat. maximo. val -> max_random-1
int emergency_threshold = 80; // el % de ocupación que provoca la activación de la tarea 

static struct proc_dir_entry *proc_entry_modtimer; // entrada de /proc
static struct proc_dir_entry *proc_entry_modconfig; // entrada de /proc

//////////////////////// Sincronisación/////////////////////////////////////////////////////////////////////////
DEFINE_SPINLOCK(sp); // mutex
unsigned long flags=0; 

DEFINE_SEMAPHORE(s_mtx);


void cleanup(void){
	struct list_item *item= NULL;
	struct list_head *cur_node=NULL;
	struct list_head *aux=NULL;
  
	if(down_interruptible(&s_mtx)) return -EINTR;
	list_for_each_safe(cur_node, aux, &my_list){
    	item=list_entry(cur_node, struct list_item, links);
    	list_del(cur_node);
    	vfree(item);
  	}
	up(&s_mtx);
}

/* Work's handler function */ 
/* La función asociada a la tarea (my_work) volcará los datos del buﬀer a la lista enlazada de enteros */
static void my_wq_function(struct work_struct *work)
{
	struct list_item *item = NULL;
	//struct list_head *cur_node = NULL;
	int size,i=0;
	unsigned int aux[CBUF_LENGTH];

	spin_lock_irqsave(&sp,flags);
	size=size_cbuffer_t(cbuffer);
	while ( i <  size ){
		aux[i]=remove_cbuffer_t(cbuffer);
		spin_unlock_irqrestore(&sp,flags);

		item=(struct list_item *) vmalloc(sizeof (struct list_item));
    	item->data = aux[i];

		///SEMAFORO s_mtx proteger la lista 
		if(down_interruptible(&s_mtx)) return -EINTR;
   		list_add_tail(&item->links, &my_list);
		up(&s_mtx);
	 	//SEMAFORO s_mtx proteger la lista
		i++;
	}	

	/*
	i=0;

	while( i < size){
		item=(struct list_item *) vmalloc(sizeof (struct list_item));
    	item->data = aux[i];
   		list_add_tail(&item->links, &my_list);
		i++;
	}
	*/
	printk(KERN_INFO "%d elements moved from the buffer to the list\n", emergency_threshold/CBUF_LENGTH);

}

/* Function invoked when timer expires (fires) */
static void fire_timer(unsigned long data)
{
	unsigned int n;
	int size;
	/* La operación módulo (%) nos da el resto de dividir n entre max_random. Este resto puede ir de 0 a max_random-1 */
	n = get_random_int() % max_random; 
	
	spin_lock_irqsave(&sp,flags); 

	/* Inserts an item at the end of the buffer */
	insert_cbuffer_t(cbuffer, n); 
	size= size_cbuffer_t(cbuffer);

	spin_unlock_irqrestore(&sp,flags);
	
	printk(KERN_INFO "Generated number: %d\n",n);
    //Consultar si un trabajo está pendiente 
	//work_pending( struct work_struct *work );
	// Hacer cuando llega al porcentaje de ocupación ?????????????????????
	if ( size >= (emergency_threshold*CBUF_LENGTH)/100 ){
		int cpu_actual = smp_processor_id(); 
		/* Enqueue work Se puede seleccionar en qué CPU */
		if (cpu_actual == 0)
			schedule_work_on(cpu_actual+1, &my_work); 	
		else
			schedule_work_on(cpu_actual-1, &my_work);
	}
        
    /* Re-activate the timer one second from now */
	mod_timer( &(my_timer), jiffies + timer_period); 
}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// MODTIMER: APERTURA,LECTURA Y CIERRE /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int open_modtimer(struct inode *node, struct file *filp) {
	
	cbuffer = create_cbuffer_t(CBUF_LENGTH);	
		
	INIT_LIST_HEAD(&my_list);

	/* Create timer */
	init_timer(&my_timer);
	/* Initialize field */
	my_timer.data=0;
	my_timer.function=fire_timer;
	my_timer.expires=jiffies + timer_period;  /* Activate it one second from now */
	/* Activate the timer for the first time */
	add_timer(&my_timer);		

	/* 
	Initialize work structure (with function) 
	Trabajo diferido "la función de la tarea volcara los datos del cbuffer a la lista enlazada  
	*/
	INIT_WORK(&my_work, my_wq_function ); 

	/* Incrementar el contador de referencias del modulo */
	try_module_get(THIS_MODULE); 
	
	return 0;
}

static ssize_t read_modtimer(struct file *filp, char __user *buf, size_t len, loff_t *off) {
	
	return len;
}

static int release_modtimer(struct inode *node, struct file *filp) {
	
	/*Vaciar el buffer circular*/
	destroy_cbuffer_t ( cbuffer );	
	
	/* Wait until completion of the timer function (if it's currently running) and delete timer */
    del_timer_sync(&my_timer);	
	
	/*Vaciar(liberar memoria) la lista_enlazada*/
	cleanup();

	/*Esperar ﬁnalización de todos los trabajos en workqueue por defecto*/
	flush_scheduled_work(); 

    /* Decrementar el contador de referencias*/
	module_put(THIS_MODULE);
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// MODCONFIG: ESCRITURA,LECTURA ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ssize_t write_modconfig(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
	
	int available_space = BUFFER_LENGTH-1;
  	char kbuf[BUFFER_LENGTH];
	int val;

	if ((*off) > 0) /* The application can write in this entry just once !! */
		return 0;
 	
	if (len > available_space) {
		printk(KERN_INFO "modtimer: not enough space!!\n");
		return -ENOSPC; // No queda espacio en el dispositivo
	}

	/* Transfer data from user to kernel space 
    ptr destino : kbuf
    ptr origen : buf
    nrbytes: len
  	*/
	if (copy_from_user( kbuf, buf, len ))  
		return -EFAULT; // Direccion incorrecta
  
	kbuf[len]='\0'; // añadimos el fin de la cadena al copiar los datos from user space.
	*off+=len;            /* Update the file pointer */
	
	/*sscanf() return : el nº de elementos asignados  strcmp() return : 0 -> si son iguales  */
	if( sscanf(kbuf,"timer_period_ms %d",&val) == 1){
		if(val < 4) // cada tick por defecto son 4 ms 
			return -1;
		timer_period=val/4; //ticks
	}
	else if( sscanf(kbuf,"max_random %d",&val) == 1){
		max_random=val;
	}
	else if ( sscanf(kbuf,"emergency_threshold %d",&val) == 1){
		if(val<10)
			return -1;
		emergency_threshold=val;
	}
	else{
		printk(KERN_INFO "ERROR: comando no valido!!!\n");
		return -1;
	}

	return len;
}

static ssize_t read_modconfig(struct file *filp, char __user *buf, size_t len, loff_t *off) {
	
	int nr_bytes;
	char kbuf[BUFFER_LENGTH] = "";
	char *list_string = kbuf;

	/* Tell the application that there is nothing left to read  "Para no copiar basura si llamas otra vez" */
	if ((*off) > 0) 
		return 0;
	
	list_string += sprintf(list_string, "timer_period_ms=%d\n", timer_period*4);
	list_string += sprintf(list_string, "max_random=%d\n", max_random);
	list_string += sprintf(list_string, "emergency_threshold=%d\n", emergency_threshold);
	
	nr_bytes = list_string-kbuf;
	
	if (len < nr_bytes)
    	return -ENOSPC; //No queda espacio en el dispositivo
  
    /* Transfer data from the kernel to userspace */  
  	if (copy_to_user(buf, kbuf, nr_bytes))
    	return -EINVAL; //Argumento invalido
    
  	(*off)+=len;  /* Update the file pointer */

  	return nr_bytes; 

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



int init_timer_module( void ){

    int ret=0;

	proc_entry_modtimer = proc_create( "modtimer", 0666, NULL, &proc_entry_modtimer_fops);
	if (proc_entry_modtimer == NULL) {
    	printk(KERN_INFO "Modtimer: Can't create /proc entry\n");
		ret = -ENOMEM; // No hay bastante espacio
  	}else {
    	printk(KERN_INFO "Modtimer: Module loaded\n");
  	}
	
	proc_entry_modconfig = proc_create( "modconfig", 0666, NULL, &proc_entry_modconfig_fops); 
	if (proc_entry_modconfig == NULL) {
    	printk(KERN_INFO "Modconfig: Can't create /proc entry\n");
		ret = -ENOMEM; // No hay bastante espacio
  	}else {
    	printk(KERN_INFO "Modconfig: Module loaded\n");
  	}	

    return ret;
}


void cleanup_timer_module( void ){
  
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
