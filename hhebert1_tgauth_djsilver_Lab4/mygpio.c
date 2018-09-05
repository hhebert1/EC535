
/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <asm/gpio.h>
/*#include <asm/hardware.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/list.h>

#include <asm-arm/arch-pxa/pxa-regs.h>
/*#include <asm-arm/arch-pxa/hardware.h>
#include <asm-arm/arch-pxa/gpio.h>
*/

MODULE_LICENSE("Dual BSD/GPL");

/* Declaration of memory.c functions */
static int mygpio_open(struct inode *inode, struct file *filp);
static int mygpio_release(struct inode *inode, struct file *filp);
static ssize_t mygpio_read(struct file *filp,
			    char *buf, size_t count, loff_t *f_pos);
static ssize_t mygpio_write(struct file *filp,
			     const char *buf, size_t count, loff_t *f_pos);
static void mygpio_callback(void);

static void mygpio_exit(void);
static int mygpio_init(void);

/* Structure that declares the usual file */
/* access functions */
struct file_operations mygpio_fops = {
 read: mygpio_read,
 write: mygpio_write,
 open: mygpio_open,
 release: mygpio_release
};

/* Declaration of the init and exit functions */
module_init(mygpio_init);
module_exit(mygpio_exit);

static unsigned capacity= 128;
static unsigned bite = 128;

int count;
int b0_pressed;
int b1_pressed;
static struct timer_list * countdown = NULL;

module_param(capacity, uint, S_IRUGO);
module_param(bite, uint, S_IRUGO);

/* Global variables of the driver */
/* Major number */
static int mygpio_major = 63;

/* Buffer to store data */
static char *mygpio_buffer;
/* length of the current message */
static int mygpio_len;

static int mygpio_init(void)
{
	int result;
	count = 15;
	
  /* Registering device */
  result = register_chrdev(mygpio_major, "mygpio", &mygpio_fops);
  if (result < 0)
    {
      printk(KERN_ALERT
	     "mygpio: cannot obtain major number %d\n", mygpio_major);
      return result;
    }

	countdown = kmalloc(sizeof(struct timer_list), GFP_KERNEL);
	if(!countdown)
	{
		printk(KERN_ALERT "Insufficient kernel memory for timer\n");
		result = -ENOMEM;
		goto fail;
	}

  /* Allocating mygpio for the buffer */
  mygpio_buffer = kmalloc(capacity, GFP_KERNEL); 
  if (!mygpio_buffer)
    { 
      printk(KERN_ALERT "Insufficient kernel memory\n"); 
      result = -ENOMEM;
      goto fail; 
    } 
  memset(mygpio_buffer, 0, capacity);
  mygpio_len = 0;
  
    // Declaring outputs, initialize to 15	
	pxa_gpio_mode(28 | GPIO_OUT);
	pxa_gpio_mode(29 | GPIO_OUT);
	pxa_gpio_mode(30 | GPIO_OUT);
	pxa_gpio_mode(31 | GPIO_OUT);
	pxa_gpio_mode(17 | GPIO_IN);
	pxa_gpio_mode(101 | GPIO_IN);

	gpio_set_value(28,1);
	gpio_set_value(29,1);
	gpio_set_value(30,1);
	gpio_set_value(31,1);


	b0_pressed = gpio_get_value(17);
  	b1_pressed = gpio_get_value(101);

	init_timer(countdown);
	countdown->expires = jiffies + HZ;
	countdown->data = 0;
	countdown->function = mygpio_callback;
	add_timer(countdown);

  printk(KERN_ALERT "Inserting mygpio module\n"); 
  return 0;

 fail: 
  mygpio_exit(); 
  return result;
}

static void mygpio_exit(void)
{
  /* Freeing the major number */
  unregister_chrdev(mygpio_major, "mygpio");

  /* Freeing buffer memory */
  if (mygpio_buffer)
    {
      kfree(mygpio_buffer);
    }
	/* free timer */
	del_timer(countdown);
	kfree(countdown);

  printk(KERN_ALERT "Removing mygpio module\n");

}

static int mygpio_open(struct inode *inode, struct file *filp)
{
  printk(KERN_INFO "open called: process id %d, command %s\n",
	 current->pid, current->comm);
  /* Success */
  return 0;
}

static int mygpio_release(struct inode *inode, struct file *filp)
{
  printk(KERN_INFO "release called: process id %d, command %s\n",
	 current->pid, current->comm);
  /* Success */
  return 0;
}

static ssize_t mygpio_read(struct file *filp, char *buf, 
			    size_t count, loff_t *f_pos)
{ 
  return 0;
}

static ssize_t mygpio_write(struct file *filp, const char *buf,
			     size_t count, loff_t *f_pos)
{
  return 0;
}

static void mygpio_callback(void)
{
	b0_pressed = gpio_get_value(17);
  	b1_pressed = gpio_get_value(101);

	if(b0_pressed)
	{
		if(b1_pressed)
		{
			count++;
			if(count > 15)
				count = 1;
		}
		else
		{
			count--;
			if(count < 1)
				count = 15;
		}
	}

		switch(count)
		{
			case 1:
			gpio_set_value(28,1);
			gpio_set_value(29,0);
			gpio_set_value(30,0);
			gpio_set_value(31,0);
			break;
			case 2:
			gpio_set_value(28,0);
			gpio_set_value(29,1);
			gpio_set_value(30,0);
			gpio_set_value(31,0);
			break;
			case 3:
			gpio_set_value(28,1);
			gpio_set_value(29,1);
			gpio_set_value(30,0);
			gpio_set_value(31,0);
			break;
			case 4:
			gpio_set_value(28,0);
			gpio_set_value(29,0);
			gpio_set_value(30,1);
			gpio_set_value(31,0);
			break;
			case 5:
			gpio_set_value(28,1);
			gpio_set_value(29,0);
			gpio_set_value(30,1);
			gpio_set_value(31,0);
			break;
			case 6:
			gpio_set_value(28,0);
			gpio_set_value(29,1);
			gpio_set_value(30,1);
			gpio_set_value(31,0);
			break;
			case 7:
			gpio_set_value(28,1);
			gpio_set_value(29,1);
			gpio_set_value(30,1);
			gpio_set_value(31,0);
			break;
			case 8:
			gpio_set_value(28,0);
			gpio_set_value(29,0);
			gpio_set_value(30,0);
			gpio_set_value(31,1);
			break;
			case 9:
			gpio_set_value(28,1);
			gpio_set_value(29,0);
			gpio_set_value(30,0);
			gpio_set_value(31,1);
			break;
			case 10:
			gpio_set_value(28,0);
			gpio_set_value(29,1);
			gpio_set_value(30,0);
			gpio_set_value(31,1);
			break;
			case 11:
			gpio_set_value(28,1);
			gpio_set_value(29,1);
			gpio_set_value(30,0);
			gpio_set_value(31,1);
			break;
			case 12:
			gpio_set_value(28,0);
			gpio_set_value(29,0);
			gpio_set_value(30,1);
			gpio_set_value(31,1);
			break;
			case 13:
			gpio_set_value(28,1);
			gpio_set_value(29,0);
			gpio_set_value(30,1);
			gpio_set_value(31,1);
			break;
			case 14:
			gpio_set_value(28,0);
			gpio_set_value(29,1);
			gpio_set_value(30,1);
			gpio_set_value(31,1);
			break;
			case 15:
			gpio_set_value(28,1);
			gpio_set_value(29,1);
			gpio_set_value(30,1);
			gpio_set_value(31,1);
			break;
		}
		
		mod_timer(countdown, jiffies + HZ);						

	}

