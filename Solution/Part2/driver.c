#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/err.h>
#include<linux/wait.h>
 

//values to read
#define PID 		0
#define	STATIC_PRIO 	1
#define	COMM 		2
#define PPID		3
#define NVCSW		4
#define DEVNAME "cs614_device"

struct linkedlist
{
int proc_id;
int csvalue ;
struct list_head node1;
};
static LIST_HEAD(head) ;
/*
** Function Prototypes
*/
static int      __init cs614_driver_init(void);
static void     __exit cs614_driver_exit(void);
 
 


static int major;
atomic_t  device_opened;
static struct class *cs614_class; 
struct device *cs614_device; 

static char *d_buf = NULL;

//for sysfs
static int cs614_value = 0;
//sysfs closed




//for char device
static int cs614_open(struct inode *inode, struct file *file)
{
        atomic_inc(&device_opened);
        try_module_get(THIS_MODULE);
        printk(KERN_INFO "Device opened successfully\n");
        return 0;
}

static int cs614_release(struct inode *inode, struct file *file)
{
        atomic_dec(&device_opened);
        module_put(THIS_MODULE);
        printk(KERN_INFO "Device closed successfully\n");
        return 0;
}

static int find_val(void)
{
       int value1;
	struct linkedlist *find;
	list_for_each_entry(find,&head,node1){
	if(find->proc_id==current->pid){
	value1=find->csvalue;

	}
	}  
	
	return value1;
}
static ssize_t cs614_read(struct file *filp,
                           char *ubuf,
                           size_t length,
                           loff_t * offset)
	{ 
	char buf[16];
	int len;
	
	
	
	if(length < 16)
		return -EINVAL;
		

	switch(find_val())
	{
	case 0:
            len = sprintf(buf, "%d\n", current->pid);
            break;
        case 1:
            len = sprintf(buf, "%d\n", current->static_prio);
            break;
        case 2:
            len= sprintf(buf,"%s", current->comm);
            break;
        case 3:
            len = sprintf(buf, "%d\n", current->real_parent->pid);
            break;
        case 4:
            len = sprintf(buf, "%lu\n", current->nvcsw);
            break;
        default:
            len = sprintf(buf, "%d\n", -EINVAL);
    }

    
    if (copy_to_user(ubuf, buf, len))
     {
        return -EINVAL;
     }
	
	
    return len;        
       
}

static ssize_t
cs614_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	   
	    printk(KERN_INFO "In write\n");
	   
	    if(copy_from_user(d_buf, buff, len))
		    return -EINVAL;
            
            return 8;
}

static struct file_operations fops = {
        .read = cs614_read,
        .write = cs614_write,
        .open = cs614_open,
        .release = cs614_release,
};



//char device closed

//for sysfs
static ssize_t cs614_status(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{	
        pr_info("sysfs - read\n");
        return sprintf(buf, "%d\n", cs614_value);
}

static void insert_new_node(int id, int value)
{
        struct linkedlist *newnode;
        newnode=kmalloc(sizeof(struct linkedlist),GFP_KERNEL) ;
        newnode->proc_id=id;
        newnode->csvalue=value;
      	list_add_tail(&newnode->node1,&head);
       
}

static ssize_t cs614_set(struct kobject *kobj,
                                   struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{	
	
	
        pr_info("sysfs - write\n");
        
        sscanf(buf,"%d",&cs614_value) ;
       insert_new_node(current->pid,cs614_value);
        return count ;
        
}

static struct kobj_attribute cs614_attribute = __ATTR(cs614_value, 0660, cs614_status, cs614_set);
static struct attribute *cs614_attrs[] = {
        &cs614_attribute.attr,
        NULL,
};
static struct attribute_group cs614_attr_group = {
        .attrs = cs614_attrs,
        .name = "cs614_sysfs",
};

//sysfs closed

//init module start

static int __init cs614_driver_init(void)
{
        int err;
        int ret = sysfs_create_group (kernel_kobj, &cs614_attr_group);
	
	
	printk(KERN_INFO "Hello kernel\n");
            
        major = register_chrdev(0, DEVNAME, &fops);
        err = major;
        if (err < 0) {      
             printk(KERN_ALERT "Registering char device failed with %d\n", major);   
             goto error_regdev;
        }                 
        
        cs614_class = class_create(THIS_MODULE, DEVNAME);
        err = PTR_ERR(cs614_class);
        if (IS_ERR(cs614_class))
                goto error_class;

        

        cs614_device = device_create(cs614_class, NULL,
                                        MKDEV(major, 0),
                                        NULL, DEVNAME);
        err = PTR_ERR(cs614_device);
        if (IS_ERR(cs614_device))
                goto error_device;
 
        d_buf = kzalloc(4096, GFP_KERNEL);
        printk(KERN_INFO "I was assigned major number %d.To talk to\n",major);                                                              
        atomic_set(&device_opened, 0);
       
       pr_info("Device Driver Insert...Done!!!\n");
       
       //sysfs file creation
       
        if(unlikely(ret))
                printk(KERN_INFO "demo: can't create sysfs\n");

	printk(KERN_INFO "cs614_sysfs directory created \n");
	
	return 0;

	

error_device:
         class_destroy(cs614_class);
error_class:
        unregister_chrdev(major, DEVNAME);
error_regdev:
        return  err;
}

static void __exit cs614_driver_exit(void)
{
	kfree(d_buf);
        device_destroy(cs614_class, MKDEV(major, 0));
        class_destroy(cs614_class);
        unregister_chrdev(major, DEVNAME);
        pr_info("Device Driver Remove...Done!!!\n");
        sysfs_remove_group (kernel_kobj, &cs614_attr_group);
       
	printk(KERN_INFO "Removed cs614_sysfs directory\n");
	printk(KERN_INFO "Goodbye kernel\n");
}

 
 
 
 
 
module_init(cs614_driver_init);
module_exit(cs614_driver_exit);

MODULE_AUTHOR("sumit@cse.iitk.ac.in");
MODULE_LICENSE("GPL");
