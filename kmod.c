#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/slab.h> 
 
static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class
static char *buffer_var;

static int vuln_open(struct inode *i, struct file *f)
{
  printk(KERN_INFO "[i] Module vuln: open()\n");
  return 0;
}
 
static int vuln_close(struct inode *i, struct file *f)
{
  printk(KERN_INFO "[i] Module vuln: close()\n");
  return 0;
}

static ssize_t vuln_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
  if(strlen(buffer_var)>0) {
    printk(KERN_INFO "[i] Module vuln read: %s\n", buffer_var);
    kfree(buffer_var);
    buffer_var=kmalloc(100,GFP_DMA);
    return 0;
  } else {
    return 1;
  }
}

static ssize_t vuln_write(struct file *f, const char __user *buf,size_t len, loff_t *off)
{
  char buffer[100]={0};

  if (_copy_from_user(buffer, buf, len))
    return -EFAULT;
  buffer[len-1]='\0';

  printk("[i] Module vuln write: %s\n", buffer);

  strncpy(buffer_var,buffer,len);

  return len;
}
 
static struct file_operations pugs_fops =
{
  .owner = THIS_MODULE,
  .open = vuln_open,
  .release = vuln_close,
  .write = vuln_write,
  .read = vuln_read
};
 
static int __init vuln_init(void) /* Constructor */
{
  buffer_var=kmalloc(100,GFP_DMA);
  printk(KERN_INFO "[i] Module vuln registered");
  if (alloc_chrdev_region(&first, 0, 1, "vuln") < 0)
  {
    return -1;
  }
  if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
  {
    unregister_chrdev_region(first, 1);
    return -1;
  }
  if (device_create(cl, NULL, first, NULL, "vuln") == NULL)
  {
    printk(KERN_INFO "[i] Module vuln error");
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }
  cdev_init(&c_dev, &pugs_fops);
  if (cdev_add(&c_dev, first, 1) == -1)
  {
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }
 
  printk(KERN_INFO "[i] <Major, Minor>: <%d, %d>\n", MAJOR(first), MINOR(first));
  return 0;
}
 
static void __exit vuln_exit(void) /* Destructor */
{
    unregister_chrdev_region(first, 3);
    printk(KERN_INFO "Module vuln unregistered");
}
 
module_init(vuln_init);
module_exit(vuln_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("blackndoor");
MODULE_DESCRIPTION("Module vuln overflow");