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
  Data* data;
  printk("enetring device_open");
  data =kmalloc(sizeof(Data),GFP_KERNEL);
  if(!data){
    printk("data memory allocation failed in device_open");
    return -ENOMEM;
  }
  data->my_channel = NULL;
  file->private_data =(void*)data;
  return 0;
}


//----------------------------------------------------------------
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset)
{
  Data* data;
  printk("enetring device_read");
  data = (Data*)(file->private_data);
  if(!data->my_channel){
    printk("Channel not set in device_read");
    return -EINVAL;
  }
  if(!data->my_channel->last_message){
    printk("No message exists in the requested channel");
    return -EWOULDBLOCK;
  }
  if(data->my_channel->message_len > length){
    printk("provided buffer length is too small");
    return -ENOSPC;
  }
  if(copy_to_user(buffer,data->my_channel->last_message,data->my_channel->message_len)!= 0){
    printk("copy_to_user failed in device_read");
    return -EFAULT;
  }
  return data->my_channel->message_len;
}


//----------------------------------------------------------------
static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset)
{
  char* message;
  Data* data;
  printk("now we access private data");
  printk(file->private_data);
  if(!file->private_data || !buffer){
    printk("private data not assigned to channel");
    return -EFAULT;
  }
  data = (Data*)(file->private_data);
  printk("enetring write");
  if(!data->my_channel){
    printk("Channel not set in device_write");
    return -EINVAL;
  }
  channel = data->my_channel
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
  if(data->my_channel->last_message){
    kfree(data->my_channel->last_message);
  }
  printk("after 5 if");
  data->my_channel->last_message = message;
  data->my_channel->message_len = (int)length;
  return length;
}

//----------------------------------------------------------------
static Channel* create_channel(unsigned int channel_id){
  Channel* channel = kmalloc(sizeof(channel), GFP_KERNEL);
  if(!channel){
    printk("new channel memory allocation failed in create_channel");
    return -ENOMEM;
  }
  channel->id = channel_id;
  channel->last_message = NULL;
  channel->message_len = 0;
  channel->next = NULL;
  return channel;
}

//----------------------------------------------------------------
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param)
{
  int minor;
  Channel* curr;
  Data* data;
  unsigned int channel_id;
  channel_id = (unsigned int)ioctl_param;
  printk("enetring device_ioctl");
  minor = (int)iminor(file->f_inode);
  if(ioctl_command_id != MSG_SLOT_CHANNEL || ioctl_param == 0){
    printk("device_icotl failed");
    return -EINVAL;
  }
  data =(Data*)(file->private_data);
  else if(!devices[minor]){
    devices[minor] = create_channel(channel_id);
    printk("icotl created head channel successfully");
    data->my_channel=devices[minor];
    return 0;
  }
  else{
    curr = devices[minor];
    while(curr->id != channel_id){
      if(!curr->next){
        curr->next =create_channel(channel_id);
      }
      curr = curr->next;
    }
    data->my_channel = curr;
    printk("icotl created channel successfully");
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