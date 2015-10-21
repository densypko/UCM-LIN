#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <linux/list.h>
#include <linux/list_sort.h>

/*Configuracion del proyecto*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Modlist Kernel Module - FDI-UCM");
MODULE_AUTHOR("Marco Cuevas, Denys Sypko");

/*Prototipos*/
void cleanup(void);
static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
int init_modlist_module( void );
void exit_modlist_module( void );
int cmp(void *priv, struct list_head *a, struct list_head *b);



//<<<<<<<<<<<<<<<<<============================>>>>>>>>>>>>>>>>>>>//
//<<<<<<<<<<<<<<<<<<<< Definiciones Globales >>>>>>>>>>>>>>>>>>>>>//
//<<<<<<<<<<<<<<<<<============================>>>>>>>>>>>>>>>>>>>//
#define BUFFER_LENGTH       100

static const struct file_operations proc_entry_fops = {
    .read = modlist_read,
    .write = modlist_write,    
};

static struct proc_dir_entry *proc_entry; // entrada de /proc
struct list_item{
  int data;
  struct list_head links;
};
static struct list_head my_list; // nodo fantasma



//<<<<<<<<<<<<<<<<<============================>>>>>>>>>>>>>>>>>>>//
//<<<<<<<<<<<<<<<<< Implementecaion de metodos >>>>>>>>>>>>>>>>>>>//
//<<<<<<<<<<<<<<<<<============================>>>>>>>>>>>>>>>>>>>//

void cleanup(void){
  struct list_item *item= NULL;
  struct list_head *cur_node=NULL;
  struct list_head *aux=NULL;
  
  list_for_each_safe(cur_node, aux, &my_list){
      item=list_entry(cur_node, struct list_item, links);
      list_del(cur_node);
      vfree(item);
  }
}

int cmp(void *priv, struct list_head *a, struct list_head *b) {
  int ret=-2;
  
  struct list_item *itemA = NULL; 
  struct list_item *itemB = NULL; 

  itemA = list_entry(a, struct list_item, links);
  itemB = list_entry(b, struct list_item, links);

  if((itemA != NULL) || (itemB != NULL)) {
    if(itemA->data < itemB->data)
      ret = -1;
    else if(itemA->data > itemB->data)
      ret = 1;
    else 
      ret = 0;
  }
  return ret;
}


static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

  int available_space = BUFFER_LENGTH-1;
  int num;
  char kbuf[BUFFER_LENGTH];

  struct list_item *item = NULL;
  struct list_head *cur_node = NULL;

  if ((*off) > 0) /* The application can write in this entry just once !! */
    return 0;
  
  if (len > available_space) {
    printk(KERN_INFO "modlist: not enough space!!\n");
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

  /*sscanf() return : el nº de elementos asignados*/
  if( sscanf(kbuf,"add %d",&num) == 1) { 
    item=(struct list_item *) vmalloc(sizeof (struct list_item));
    item->data = num;
    //static inline void list_add_tail(struct list_head *new, struct list_head *head)
    list_add_tail(&item->links, &my_list);
  }else if( sscanf(kbuf,"remove %d",&num) == 1) { 
    struct list_head *aux = NULL;
    // iterrar sobre una lista de forma segura para no eliminar la entrada
    list_for_each_safe(cur_node, aux, &my_list) { 
      /* item points to the structure wherein the links are embedded */ 
      item = list_entry(cur_node, struct list_item, links);
      if((item->data) == num){
        list_del(cur_node);
        vfree(item);
      }
    }
  }else if ( strcmp(kbuf,"cleanup\n") == 0){ // strcmp() return : 0 -> si son iguales 
    cleanup();
  }else if(strcmp(kbuf,"sort\n") == 0) {
    /**
     * list_sort - sort a list
     * @priv: private data, opaque to list_sort(), passed to @cmp
     * @head: the list to sort
     * @cmp: the elements comparison function
     *
     * This function implements "merge sort", which has O(nlog(n))
     * complexity.
     *
     * The comparison function @cmp must return a negative value if @a
     * should sort before @b, and a positive value if @a should sort after
     * @b. If @a and @b are equivalent, and their original relative
     * ordering is to be preserved, @cmp must return 0.
    */
    list_sort(NULL, &my_list, *cmp);
  }else{
    printk(KERN_INFO "ERROR: comando no valido!!!\n");
  }
  return len;
}

static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
  
  int nr_bytes;
  struct list_item *item = NULL; 
  struct list_head *cur_node = NULL; 
  
  char kbuf[BUFFER_LENGTH] = "";
  char *list_string = kbuf;


  if ((*off) > 0) /* Tell the application that there is nothing left to read  "Para no copiar basura si llamas otra vez" */
      return 0;
  
  
  list_for_each(cur_node, &my_list) { 
    /* item points to the structure wherein the links are embedded */ 
    item = list_entry(cur_node, struct list_item, links);
    list_string += sprintf(list_string, "%d\n", item->data);
  }

  nr_bytes=list_string-kbuf;
    
  if (len<nr_bytes)
    return -ENOSPC; //No queda espacio en el dispositivo
  
    /* Transfer data from the kernel to userspace */  
  if (copy_to_user(buf, kbuf, nr_bytes))
    return -EINVAL; //Argumento invalido
    
  (*off)+=len;  /* Update the file pointer */
  //vfree(kbuf);
  return nr_bytes; 
}





int init_modlist_module( void )
{
  int ret = 0;
  INIT_LIST_HEAD(&my_list);

  /* en modlist defenido en /proc, solo podemos usar las funciones defenidas en proc_entry_fops */
  proc_entry = proc_create( "modlist", 0666, NULL, &proc_entry_fops); 
  if (proc_entry == NULL) {
    ret = -ENOMEM; // No hay bastante espacio
    printk(KERN_INFO "Modlist: Can't create /proc entry\n");
  }else {
    printk(KERN_INFO "Modlist: Module loaded\n");
  }

  return ret;

}


void exit_modlist_module( void )
{
  cleanup();
  remove_proc_entry("modlist", NULL); // eliminar la entrada del /proc
  printk(KERN_INFO "Modlist: Module unloaded.\n");
}

module_init( init_modlist_module );
module_exit( exit_modlist_module );
