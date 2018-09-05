
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
#include <linux/interrupt.h>
#include <asm/arch/pxa-regs.h>
#include <asm/hardware.h>
#include <linux/jiffies.h>
#include <linux/timer.h>

MODULE_LICENSE("Dual BSD/GPL");

/* Declaration of memory.c functions */
static int mygpio_open(struct inode *inode, struct file *filp);
static int mygpio_release(struct inode *inode, struct file *filp);
static ssize_t mygpio_read(struct file *filp,
			    char *buf, size_t count, loff_t *f_pos);
static ssize_t mygpio_write(struct file *filp,
			     const char *buf, size_t count, loff_t *f_pos);
static void mygpio_callback(void);
static void debounce_callback(void);

static void mygpio_exit(void);
static int mygpio_init(void);

static void backlight_power(int dutyCycle);


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

int tcount;
static struct timer_list * countdown = NULL;
static struct timer_list * debounce = NULL;
int brightness;

module_param(capacity, uint, S_IRUGO);
module_param(bite, uint, S_IRUGO);

int b0_pressed;
int b1_pressed;
int b2_pressed;

int up_down;
int on_off;
int frequency;

static int mygpio_major = 63;
static char *mygpio_buffer;
static int mygpio_len;

int irqNum1;
int irqNum2; 
int irqNum3;

static irq_handler_t gpio1_irq(int irq, void *dev_id, struct pt_regs *regs);
static irq_handler_t gpio2_irq(int irq, void *dev_id, struct pt_regs *regs);
static irq_handler_t gpio3_irq(int irq, void *dev_id, struct pt_regs *regs);

static irq_handler_t gpio1_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	//no debounce, active for both edges
	//printk("Button0 IRQ\n");
	//debounce->expires = jiffies + msecs_to_jiffies(150);
	mod_timer(debounce, jiffies+ msecs_to_jiffies(150));
	return (irq_handler_t)IRQ_HANDLED;
}

static irq_handler_t gpio2_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	//no debounce, active for both edges
	//printk("Button1 IRQ\n");
	mod_timer(debounce, jiffies+ msecs_to_jiffies(150));
	//debounce->expires = jiffies + msecs_to_jiffies(150);
	return (irq_handler_t)IRQ_HANDLED;
}

static irq_handler_t gpio3_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	//no debounce, active for both edges
	//printk("Button2 IRQ\n");
	//mod_timer(debounce, jiffies+ msecs_to_jiffies(150));
	brightness++;
	if(brightness == 4)
	{
		brightness =1;
	}
	//backlight_power(brightness);
	return (irq_handler_t)IRQ_HANDLED;
}


static int mygpio_init(void)
{
	int result;
	tcount = 15;
	brightness = 1;
	frequency = 1;
	up_down = 1;
	on_off = 0;

	b0_pressed = 0;
	b1_pressed = 0;
	b2_pressed = 0;
	
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
	debounce = kmalloc(sizeof(struct timer_list), GFP_KERNEL);
	if(!debounce)
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
	pxa_gpio_mode(16 | GPIO_OUT);
	pxa_gpio_mode(29 | GPIO_OUT);
	pxa_gpio_mode(30 | GPIO_OUT);
	pxa_gpio_mode(31 | GPIO_OUT);
	pxa_gpio_mode(9 | GPIO_IN);
	pxa_gpio_mode(101 | GPIO_IN);
	pxa_gpio_mode(28 | GPIO_IN);

	gpio_set_value(16,1);
	gpio_set_value(29,1);
	gpio_set_value(30,1);
	gpio_set_value(31,1);

	/*init_timer(countdown);
	countdown->expires = jiffies + HZ;
	countdown->data = 0;
	countdown->function = mygpio_callback;
	add_timer(countdown);

	init_timer(debounce);
	debounce->expires = jiffies + msecs_to_jiffies(150);
	debounce->data = 0;
	debounce->function = debounce_callback;
	add_timer(debounce);*/

	setup_timer(countdown, mygpio_callback, 0);
	setup_timer(debounce, debounce_callback, 0);
	
	mod_timer(countdown, jiffies+ msecs_to_jiffies(frequency*1000));

	//interrupts
	irqNum1 = gpio_to_irq(9);
	irqNum2 = gpio_to_irq(28);
	irqNum3 = gpio_to_irq(101);

	result = request_irq(irqNum1, (irq_handler_t) gpio1_irq, IRQF_TRIGGER_RISING, "mygpio1", NULL);
	result = request_irq(irqNum2, (irq_handler_t) gpio2_irq, IRQF_TRIGGER_RISING, "mygpio2", NULL);
	result = request_irq(irqNum3, (irq_handler_t) gpio3_irq, IRQF_TRIGGER_RISING, "mygpio3", NULL);

	/*if (request_irq(irqNum1, &gpio1_irq, SA_INTERRUPT | SA_TRIGGER_RISING,"mygpio1", NULL) != 0 ) 
	{
                printk ( "irq1 not acquired \n" );
                return -1;
        }else{
                printk ( "irq1 %d acquired successfully \n", irqNum1 );
	}
	if (request_irq(irqNum2, &gpio2_irq, SA_INTERRUPT | SA_TRIGGER_RISING,"mygpio2", NULL) != 0 ) 
	{
                printk ( "irq2 not acquired \n" );
                return -1;
        }else{
                printk ( "irq2 %d acquired successfully \n", irqNum1 );
	}
	if (request_irq(irqNum3, &gpio3_irq, SA_INTERRUPT | SA_TRIGGER_RISING,"mygpio3", NULL) != 0 ) 
	{
                printk ( "irq3 not acquired \n" );
                return -1;
        }else{
                printk ( "irq3 %d acquired successfully \n", irqNum1 );
	}*/

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

	free_irq(irqNum1,NULL);
	free_irq(irqNum2,NULL);
	free_irq(irqNum3,NULL);
	
	brightness = 0;
	backlight_power(brightness);

  printk(KERN_ALERT "Removing mygpio module\n");

}

static int mygpio_open(struct inode *inode, struct file *filp)
{
  return 0;
}

static int mygpio_release(struct inode *inode, struct file *filp)
{
  return 0;
}

static ssize_t mygpio_read(struct file *filp, char *buf, 
			    size_t count, loff_t *f_pos)
{
	char str[256];
	int len;
	char direction[6];
	char on_off_state[6];
	char brightLevel[3];
	char speed[3];

	if(up_down==0)
	{
		strcpy(direction,"Down");
	}
	else
	{
		strcpy(direction,"Up");
	}

	if(on_off == 0)
	{
		strcpy(on_off_state,"Off");
	}
	else
	{
		strcpy(on_off_state,"On");
	}

	if(brightness==1)
	{
		strcpy(brightLevel,"L");
	}
	else if(brightness == 2)
	{
		strcpy(brightLevel,"M");
	}
	else if(brightness == 3)
	{
		strcpy(brightLevel,"H");
	}
	
	if(frequency<3)
	{
		strcpy(speed,"H");
	}
	else if(frequency == 3)
	{
		strcpy(speed,"M");
	}
	else if(frequency > 3)
	{
		strcpy(speed,"L");
	}	

	len = sprintf(str, "%d\t%s\t%s\t%s\t%s\t\n", tcount, speed, on_off_state, direction, brightLevel);

	if (*f_pos >= len)
		return 0;

	if (copy_to_user(buf, str, 256)) 
		return -EFAULT;

	*f_pos += len;
	return len;	
}

static ssize_t mygpio_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	if (copy_from_user(mygpio_buffer, buf, count))
		return -EFAULT;

	// Validate input is "fL" (low speed), "fM" (medium speed), or "fH" (high speed) 
	if(mygpio_buffer[0] == 'f' && mygpio_buffer[1]== 'L' )
	{
		frequency = 5;	
	}
	else if(mygpio_buffer[0] == 'f' && mygpio_buffer[1]== 'M')
	{
		frequency = 3;	
	}
	else if(mygpio_buffer[0] == 'f' && mygpio_buffer[1]== 'H' )
	{
		frequency = 1;	
	}
	// ...other valid input is "vN" with N = 0,1,2,3...c,d,e,f
	else if(mygpio_buffer[0] == 'v' && ((mygpio_buffer[1]	>= 48 && mygpio_buffer[1] <= 57) || (mygpio_buffer[1]  >= 97 && mygpio_buffer[1] <= 102)))
	{		
		// update count 
		if (mygpio_buffer[1] >= 48 && mygpio_buffer[1] <= 57)
		{
		    tcount = (int) mygpio_buffer[1];
		}
		else
		{
		    //the number N is a, b, c, d, e, or f
		    switch(mygpio_buffer[1])
		    {
		        case 97:
		            tcount = 10;
		            break;
		        case 98:
		            tcount = 11;
		            break;
		        case 99:
		            tcount = 12;
		            break;
		        case 100:
		            tcount = 13;
		            break;
		        case 101:
		            tcount = 14;
		            break;
		        case 102:
		            tcount = 15;
		            break;
		    }
		}     
		switch(tcount)
		{
			case 0:
			gpio_set_value(16,0);
			gpio_set_value(29,0);
			gpio_set_value(30,0);
			gpio_set_value(31,0);			
			case 1:
			gpio_set_value(16,1);
			gpio_set_value(29,0);
			gpio_set_value(30,0);
			gpio_set_value(31,0);
			break;
			case 2:
			gpio_set_value(16,0);
			gpio_set_value(29,1);
			gpio_set_value(30,0);
			gpio_set_value(31,0);
			break;
			case 3:
			gpio_set_value(16,1);
			gpio_set_value(29,1);
			gpio_set_value(30,0);
			gpio_set_value(31,0);
			break;
			case 4:
			gpio_set_value(16,0);
			gpio_set_value(29,0);
			gpio_set_value(30,1);
			gpio_set_value(31,0);
			break;
			case 5:
			gpio_set_value(16,1);
			gpio_set_value(29,0);
			gpio_set_value(30,1);
			gpio_set_value(31,0);
			break;
			case 6:
			gpio_set_value(16,0);
			gpio_set_value(29,1);
			gpio_set_value(30,1);
			gpio_set_value(31,0);
			break;
			case 7:
			gpio_set_value(16,1);
			gpio_set_value(29,1);
			gpio_set_value(30,1);
			gpio_set_value(31,0);
			break;
			case 8:
			gpio_set_value(16,0);
			gpio_set_value(29,0);
			gpio_set_value(30,0);
			gpio_set_value(31,1);
			break;
			case 9:
			gpio_set_value(16,1);
			gpio_set_value(29,0);
			gpio_set_value(30,0);
			gpio_set_value(31,1);
			break;
			case 10:
			gpio_set_value(16,0);
			gpio_set_value(29,1);
			gpio_set_value(30,0);
			gpio_set_value(31,1);
			break;
			case 11:
			gpio_set_value(16,1);
			gpio_set_value(29,1);
			gpio_set_value(30,0);
			gpio_set_value(31,1);
			break;
			case 12:
			gpio_set_value(16,0);
			gpio_set_value(29,0);
			gpio_set_value(30,1);
			gpio_set_value(31,1);
			break;
			case 13:
			gpio_set_value(16,1);
			gpio_set_value(29,0);
			gpio_set_value(30,1);
			gpio_set_value(31,1);
			break;
			case 14:
			gpio_set_value(16,0);
			gpio_set_value(29,1);
			gpio_set_value(30,1);
			gpio_set_value(31,1);
			break;
			case 15:
			gpio_set_value(16,1);
			gpio_set_value(29,1);
			gpio_set_value(30,1);
			gpio_set_value(31,1);
			break;
		}
		if (tcount % 2 == 0)
		{
			backlight_power(0);
		}
		else
		{
			backlight_power(brightness);
		}
	}
	else
	{
		printk(KERN_INFO"Invalid Input. bad.\n");
		return -EINVAL;
	}

	kfree(mygpio_buffer);
	return count;
}

static void mygpio_callback(void)
{
	//b0_pressed = gpio_get_value(9);
  	//b1_pressed = gpio_get_value(101);

	if(on_off)
	{
		if(up_down)
		{
			tcount++;
			if(tcount > 15)
				tcount = 0;
		}
		else
		{
			tcount--;
			if(tcount < 0)
				tcount = 15;
		}
	}

	switch(tcount)
	{
		case 0:
		gpio_set_value(16,0);
		gpio_set_value(29,0);
		gpio_set_value(30,0);
		gpio_set_value(31,0);			
		case 1:
		gpio_set_value(16,1);
		gpio_set_value(29,0);
		gpio_set_value(30,0);
		gpio_set_value(31,0);
		break;
		case 2:
		gpio_set_value(16,0);
		gpio_set_value(29,1);
		gpio_set_value(30,0);
		gpio_set_value(31,0);
		break;
		case 3:
		gpio_set_value(16,1);
		gpio_set_value(29,1);
		gpio_set_value(30,0);
		gpio_set_value(31,0);
		break;
		case 4:
		gpio_set_value(16,0);
		gpio_set_value(29,0);
		gpio_set_value(30,1);
		gpio_set_value(31,0);
		break;
		case 5:
		gpio_set_value(16,1);
		gpio_set_value(29,0);
		gpio_set_value(30,1);
		gpio_set_value(31,0);
		break;
		case 6:
		gpio_set_value(16,0);
		gpio_set_value(29,1);
		gpio_set_value(30,1);
		gpio_set_value(31,0);
		break;
		case 7:
		gpio_set_value(16,1);
		gpio_set_value(29,1);
		gpio_set_value(30,1);
		gpio_set_value(31,0);
		break;
		case 8:
		gpio_set_value(16,0);
		gpio_set_value(29,0);
		gpio_set_value(30,0);
		gpio_set_value(31,1);
		break;
		case 9:
		gpio_set_value(16,1);
		gpio_set_value(29,0);
		gpio_set_value(30,0);
		gpio_set_value(31,1);
		break;
		case 10:
		gpio_set_value(16,0);
		gpio_set_value(29,1);
		gpio_set_value(30,0);
		gpio_set_value(31,1);
		break;
		case 11:
		gpio_set_value(16,1);
		gpio_set_value(29,1);
		gpio_set_value(30,0);
		gpio_set_value(31,1);
		break;
		case 12:
		gpio_set_value(16,0);
		gpio_set_value(29,0);
		gpio_set_value(30,1);
		gpio_set_value(31,1);
		break;
		case 13:
		gpio_set_value(16,1);
		gpio_set_value(29,0);
		gpio_set_value(30,1);
		gpio_set_value(31,1);
		break;
		case 14:
		gpio_set_value(16,0);
		gpio_set_value(29,1);
		gpio_set_value(30,1);
		gpio_set_value(31,1);
		break;
		case 15:
		gpio_set_value(16,1);
		gpio_set_value(29,1);
		gpio_set_value(30,1);
		gpio_set_value(31,1);
		break;
	}

	if (tcount % 2 == 0)
	{
		backlight_power(0);
	}
	else
	{
		backlight_power(brightness);
	}

	mod_timer(countdown, jiffies + msecs_to_jiffies((frequency)*1000));
}

static void debounce_callback(void)
{

	
	if(gpio_get_value(9) && gpio_get_value(28))
	{
		tcount = 15;
		up_down = 0;
		on_off = 0;
		gpio_set_value(16,1);
		gpio_set_value(29,1);
		gpio_set_value(30,1);
		gpio_set_value(31,1);
		backlight_power(brightness);		
	}
	else if(gpio_get_value(9)== 0 && gpio_get_value(28))
	{
		
		//printk("button 1 is pressed\n");
		if(up_down)
		{
			up_down = 0;
		}
		else
		{
			up_down = 1;
		}
	}
	else if(gpio_get_value(9) && gpio_get_value(28)==0)
	{
		//printk("button 0 is pressed\n");
		if(on_off)
		{
			on_off = 0;
		}
		else
		{
			on_off = 1;
		}
	}
	
}
static void backlight_power(int dutyCycle)
{
	pxa_gpio_mode(GPIO16_PWM0_MD);
	//pxa_set_cken(CKEN0_PWM0,1);
	CKEN |= CKEN0_PWM0;	
	PWM_CTRL0 = 0x02;
	if(dutyCycle == 1)
	{
		PWM_PWDUTY0 = 0x5;
	}
	else if(dutyCycle == 2 )
	{
		PWM_PWDUTY0 = 0x19;
	}
	else if(dutyCycle == 3 )
	{
		PWM_PWDUTY0 = 0x280;
	}
	else if(dutyCycle == 0)
	{
		PWM_PWDUTY0= 0x000;
	}

	PWM_PERVAL0 = 0x3ff;
}

