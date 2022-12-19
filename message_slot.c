#undef __KERNEL__
#define __KERNEL__

#undef MODULE
#define MODULE

/*I used recreation 6 as template for this assignment*/

#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/slab.h>
/*#include <linux/init.h>*/

MODULE_LICENSE("GPL");

#include "message_slot.h"

/* devices array:
 * devices[i] represents message slot device file with minor number i,
 * in the cell there's a pointer to a linked list of the channels of the relevant message slot device file
 */
static Channel* devices[256]; /* message slots file array indexed by minor numbers, each message slot file points to it's channel linked list */

//================== DEVICE FUNCTIONS ===========================
static int device_open(struct inode* inode, struct file*  file)
{
  printk("enetring device_open");
  file->private_data = NULL;
  return 0;
}


//----------------------------------------------------------------
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset)
{
  Channel *channel;
  printk("enetring device_read");
  channel = (Channel*)(file->private_data);
  if(!channel){
    printk("Channel not set in device_read");
    return -EINVAL;
  }
  if(!channel->last_message){
    printk("No message exists in the requested channel");
    return -EWOULDBLOCK;
  }
  if(channel->message_len > length){
    printk("provided buffer length is too small");
    return -ENOSPC;
  }
  if(copy_to_user(buffer,channel->last_message,channel->message_len)!= 0){
    printk("copy_to_user failed in device_read");
    return -EFAULT;
  }
  return channel->message_len;
}


//----------------------------------------------------------------
static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset)
{
  char* message;
  Channel *channel;
  channel = (Channel*)(file->private_data);
  printk("enetring write");
  if(!channel){
    printk("Channel not set in device_write");
    return -EINVAL;
  }
  printk("after 1 if");
  if(length <= 0 || length > BUFF_LEN){
    printk("passed message length invalid");
    return -EMSGSIZE;
  }
  printk("after 2 if");
  message = kmalloc(sizeof(char)*length,GFP_KERNEL);
  if(!message){
    printk("message memory allocation failed in device_write");
    return -ENOMEM;
  }
  printk("after 3 if");
  if(copy_from_user(message,buffer,length)!=0){
    printk("copy_from_user failed in device_write");
    return -EFAULT;
  }
  printk("after 4 if");
  if(channel->last_message){
    kfree(channel->last_message);
  }
  printk("after 5 if");
  channel->last_message = message;
  channel->message_len = (int)length;
  return length;
}

//----------------------------------------------------------------
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param)
{
  int minor;
  unsigned int channel_id;
  channel_id = (unsigned int)ioctl_param;
  printk("enetring device_ioctl");
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
    devices[minor]->message_len = 0;
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
        curr->next->id = channel_id;
        curr->next->last_message = NULL;
        curr->next->message_len = 0;
        curr->next->next = NULL;
        curr = curr->next;
      }
      curr = curr->next;
    }
    file->private_data = curr;
    return 0;
  }
}

//==================== DEVICE SETUP =============================
// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  //.release        = device_release,??
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init init(void)
{
  int rc = -1;
  rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );
  if( rc < 0 ) {
    printk("registration failed");
    return rc;
  }
  printk( "Registeration is successful! Major num: %d\n",MAJOR_NUM);
  return 0;
}
//---------------------------------------------------------------
static void __exit simple_exit(void)
{
  // Unregister the device
  // Should always succeed
  int i;
  Channel *curr, *temp;
  printk("enetring simple_exit");
  for(i = 0; i < 256; i++){
    curr = devices[i];
    while(curr){
      temp = curr->next;
      if(curr->last_message){
        kfree(curr->last_message);
      }
      kfree(curr);
      curr = temp;
    }
  }
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

module_init(init);
module_exit(simple_exit);
//========================= END OF FILE =========================