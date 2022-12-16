/*I used recreation 6 as template for this assignment*/
#undef __KERNEL__
#define __KERNEL__

#undef MODULE
#define MODULE


#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/slab.h>

MODULE_LICENSE("GPL");
#include "message_slot.h"

/* devices array:
 * devices[i] represents message slot device file with minor number i,
 * in the cell there's a pointer to a linked list of the channels of the relevant message slot device file
 */
static Channel* devices[256]; /* message slots file array indexed by minor numbers, each message slot file points to it's channel linked list */

static int device_open(struct inode* inode, struct file*  file)
{
  file->private_data = NULL;
  return 0;
}


static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset)
{
  
}


static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset)
{
  int i;
  
  if(!file->private_data){
    printk("Channel not set in device_write");
    return -EINVAL;
  }
  if(length == 0 || length > BUFF_LEN){
    printk("passed message length invalid");x
    return -EMSGSIZE;
  }
  

}

static int device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param)
{
  int minor;
  unsigned int channel_id = (unsigned int)ioctl_param;
  minor = (int)iminor(file->f_inode);
  if(ioctl_command_id != MSG_SLOT_CHANNEL || ioctl_param == 0){
    printk("device_icotl failed");
    return -EINVAL;
  }
  else if(!devices[minor]){
    devices[minor] = (Channel*)kmalloc(sizeof(Channel),GFP_KERNEL);
    if(!devices[minor]){
      printk("head channel memory allocation failed in device_icotl");
      return -ENOMEM;
    }
    devices[minor]->id = channel_id;
    devices[minor]->last_message = NULL;
    devices[minor]->next = NULL;
    return 0;
  }
  else{
    Channel* curr = devices[minor];
    while(curr->id != channel_id){
      if(!curr->next){
        curr->next =(Channel*)kmalloc(sizeof(Channel),GFP_KERNEL);
        if(!curr->next){
          printk("channel memory allocation failed in device_icotl");
          return -ENOMEM;
        }
        curr->next.id = channel_id;
        curr->next.last_message = NULL;
        curr->next.next = NULL;
        curr = curr->next;
      }
      curr = curr->next;
    }
    file->private_data = (void*)curr;
    return 0;
  }
}

/* 
 * Device setup:
 * This structure will hold the functions to be called
 * when a process does something to the device we created
 */
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  //.release        = device_release,
};

static int __init init(void)
{
  int rc = -1;
  // init dev struct
  memset( &device_info, 0, sizeof(struct chardev_info) );
  spin_lock_init( &device_info.lock );

  // Register driver capabilities. Obtain major num
  rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );

  // Negative values signify an error
  if( rc < 0 ) {
    printk( KERN_ALERT "%s registraion failed for  %d\n",
                       DEVICE_FILE_NAME, MAJOR_NUM );
    return rc;
  }

  printk( "Registeration is successful. ");
  printk( "If you want to talk to the device driver,\n" );
  printk( "you have to create a device file:\n" );
  printk( "mknod /dev/%s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM );
  printk( "You can echo/cat to/from the device file.\n" );
  printk( "Dont forget to rm the device file and "
          "rmmod when you're done\n" );

  return 0;
}

//---------------------------------------------------------------
static void __exit exit(void)
{
  // Unregister the device
  // Should always succeed
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);