
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
#include <asm/hardware.h>
#include <asm/uaccess.h>
#include <asm/gpio.h>
#include <asm/arch/pxa-regs.h>
#include <linux/string.h>
#include <linux/jiffies.h> /* jiffies */
#include <linux/random.h>
#include <linux/interrupt.h>

MODULE_LICENSE("Dual BSD/GPL");

/* Declaration of memory.c functions */
static int mysnake_open(struct inode *inode, struct file *filp);
static int mysnake_release(struct inode *inode, struct file *filp);
static ssize_t mysnake_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t mysnake_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static void mysnake_exit(void);
static int mysnake_init(void);

//declare timer and timer function
static void snaketimer_callback(unsigned long data);
static void debounce_callback(unsigned long data);
static struct timer_list * snaketimer;
static struct timer_list * debounce;

//test functions
//static void mygpio_callback(void);

//declare helping functions
static void end_function(void);
static int randpoint_finder(int max);
static void start(void);

/* Structure that declares the usual file */
/* access functions */
struct file_operations mysnake_fops = {
	read: mysnake_read,
	write: mysnake_write,
	open: mysnake_open,
	release: mysnake_release
};

/* Declaration of the init and exit functions */
module_init(mysnake_init);
module_exit(mysnake_exit);

static unsigned capacity = 1024;
static unsigned bite = 1024;
module_param(capacity, uint, S_IRUGO);
module_param(bite, uint, S_IRUGO);

/* Global variables of the driver */
/* Major number */
static int mysnake_major = 63;
/* Buffer to store data */
static char *mysnake_buffer;
/* length of the current message */
static int mysnake_len;

//initialize variables
static int period = 700; //milliseconds
static int snake_length;
static int timer_count;
static int gesture_direction; 
static int width = 29;//change GUI_Array
static int height = 16;//change GUI_Array
static int GUI_Array[29][16];
static int pos_snakehead[2];
static int pos_snaketail[2];
static int pos_randpoint[2];

//test initializers
//static struct timer_list * countdown = NULL;

int b0_pressed;
int b1_pressed;
int b2_pressed;
int b3_pressed;
int irqNum1;
int irqNum2; 
int irqNum3;
int irqNum4;
int ii;
int jj;
int kk;
int ll;
//int p;
//int q;

int m;
int ptNum;
int h; //index for read function
int numPoints = 0;

int flag = 0;

static struct qtPoints
{
	int qWidth;
	int qHeight;
}qtPoints; //change GUI_Array

static struct qtPoints qPoints[414];

static irq_handler_t gpio1_irq(int irq, void *dev_id, struct pt_regs *regs);
static irq_handler_t gpio2_irq(int irq, void *dev_id, struct pt_regs *regs);
static irq_handler_t gpio3_irq(int irq, void *dev_id, struct pt_regs *regs);
static irq_handler_t gpio4_irq(int irq, void *dev_id, struct pt_regs *regs);

//test irq functions
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
	//printk("Button0 IRQ\n");
	//debounce->expires = jiffies + msecs_to_jiffies(150);
	mod_timer(debounce, jiffies+ msecs_to_jiffies(150));
	return (irq_handler_t)IRQ_HANDLED;
}
static irq_handler_t gpio3_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	//no debounce, active for both edges
	//printk("Button0 IRQ\n");
	//debounce->expires = jiffies + msecs_to_jiffies(150);
	mod_timer(debounce, jiffies+ msecs_to_jiffies(150));
	return (irq_handler_t)IRQ_HANDLED;
}

static irq_handler_t gpio4_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	//no debounce, active for both edges
	//printk("Button0 IRQ\n");
	//debounce->expires = jiffies + msecs_to_jiffies(150);
	mod_timer(debounce, jiffies+ msecs_to_jiffies(150));
	return (irq_handler_t)IRQ_HANDLED;
}

static int mysnake_init(void)
{
	int result;
	int i;
	int j;
	int k;
	int l;
	h = 0;

	//test buttons
	b0_pressed = 0;
	b1_pressed = 0;
	b2_pressed = 0;
	b3_pressed = 0;
	
	/* Registering device */
	result = register_chrdev(mysnake_major, "mysnake", &mysnake_fops);
	if (result < 0)
	{
		printk(KERN_ALERT
			"mysnake: cannot obtain major number %d\n", mysnake_major);
		return result;
	}

	/* Allocating mysnake for the buffer */
	mysnake_buffer = kmalloc(capacity, GFP_KERNEL); 
	if (!mysnake_buffer)
	{ 
		printk(KERN_ALERT "Insufficient kernel memory\n"); 
		result = -ENOMEM;
		goto fail; 
	} 
	memset(mysnake_buffer, 0, capacity);
	mysnake_len = 0;

	//initialize test timer
	snaketimer = kmalloc(sizeof(struct timer_list), GFP_KERNEL);
	if(!snaketimer)
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
	
	//intialize GUI_Array to 0
	for (k = 0; k<height; k++)
	{
		for (l = 0; l<width; l++)
		{
			GUI_Array[l][k] = 0;
		}
	}
		
	//set border, initial snake position, and initial random point in array and position values
	for (i = 0; i<height; i++)
	{
		GUI_Array[0][i] = 3;
		GUI_Array[width-1][i] = 3;
	}
	for (j = 0; j<width; j++)
	{
		GUI_Array[j][0] = 3;
		GUI_Array[j][height-1] = 3;
	}
	pos_snakehead[0] = width/2 - 1;
	pos_snakehead[1] = height/2 - 1;
	pos_snaketail[0] = width/2 - 1;
	pos_snaketail[1] = height/2 - 1;
	snake_length = 1;

	GUI_Array[pos_snakehead[0]][pos_snakehead[1]]=1;
	pos_randpoint[0] = randpoint_finder(width-1);
	pos_randpoint[1] = randpoint_finder(height-1);
	GUI_Array[pos_randpoint[0]][pos_randpoint[1]]=2;
	gesture_direction = 0;
	
	//initialize timer
	setup_timer(snaketimer, snaketimer_callback, 0);
	mod_timer(snaketimer, jiffies + msecs_to_jiffies(period));
	timer_count = 0;
	
	//test gpio modes
	pxa_gpio_mode(9 | GPIO_IN); //down b0
	pxa_gpio_mode(101 | GPIO_IN); //up b1
	pxa_gpio_mode(28 | GPIO_IN); //right b2
	pxa_gpio_mode(31 | GPIO_IN); //left b3
	
	//setup_timer(countdown, mygpio_callback, 0);
	setup_timer(debounce, debounce_callback, 0);
	mod_timer(debounce, jiffies + msecs_to_jiffies(150));

	//mod_timer(countdown, jiffies+ msecs_to_jiffies(frequency*1000));

	//test interrupts
	irqNum1 = gpio_to_irq(9);
	irqNum2 = gpio_to_irq(101);
	irqNum3 = gpio_to_irq(28);
	irqNum4 = gpio_to_irq(31);
	result = request_irq(irqNum1, (irq_handler_t) gpio1_irq, IRQF_TRIGGER_RISING, "mygpio1", NULL);
	result = request_irq(irqNum2, (irq_handler_t) gpio2_irq, IRQF_TRIGGER_RISING, "mygpio2", NULL);
	result = request_irq(irqNum3, (irq_handler_t) gpio3_irq, IRQF_TRIGGER_RISING, "mygpio3", NULL);
	result = request_irq(irqNum4, (irq_handler_t) gpio4_irq, IRQF_TRIGGER_RISING, "mygpio4", NULL);


	printk(KERN_ALERT "Inserting mysnake module\n"); 
	return 0;

fail: 
	mysnake_exit(); 
	return result;
}

static void mysnake_exit(void)
{
	/* Freeing the major number */
	unregister_chrdev(mysnake_major, "mysnake");

	/* Freeing buffer memory */
	if (mysnake_buffer)
	{
		kfree(mysnake_buffer);
	}
	
	del_timer(snaketimer);
	kfree(snaketimer);
	
	//free test stuff
	//del_timer(countdown);
	//kfree(countdown);
	del_timer(debounce);
	kfree(debounce);

	free_irq(irqNum1,NULL);
	free_irq(irqNum2,NULL);
	free_irq(irqNum3,NULL);
	free_irq(irqNum4,NULL);
	
	printk(KERN_ALERT "Removing mysnake module\n");

}

static int mysnake_open(struct inode *inode, struct file *filp)
{
	//printk(KERN_INFO "open called: process id %d, command %s\n",
	//	current->pid, current->comm);
	/* Success */
	return 0;
}

static int mysnake_release(struct inode *inode, struct file *filp)
{
	//printk(KERN_INFO "release called: process id %d, command %s\n",
	//	current->pid, current->comm);
	/* Success */
	return 0;
}

static ssize_t mysnake_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{ 

	int temp;
	char tbuf[1024], *tbptr = tbuf;
	char wholebuf[1024] = {""}, *wholeStr = wholebuf;
	int i;
	//int j;
	int num = 0;
	*f_pos -=1;
	// end of buffer reached 
	if (*f_pos >= mysnake_len)
	{	
		return 0;
	}

	// do not go over then end 
	if (count > mysnake_len - *f_pos)
		count = mysnake_len - *f_pos;

	// do not send back more than a bite 
	if (count > bite)
		count = bite;

	/* Transfering data to user space */ 

	tbptr += sprintf(tbptr,"read called: process id %d, command %s, count %d, chars ", current->pid, current->comm, count);

	for (temp = *f_pos; temp < count + *f_pos; temp++)					  
		tbptr += sprintf(tbptr, "%c", mysnake_buffer[temp]);

	//printk(KERN_INFO "%s\n", tbuf);

	// Changing reading position as best suits  
	*f_pos += count + 1; 

	//specific number of Points to print
	for(i = 0; i < 414; i++)
	{
		if(qPoints[i].qWidth != 0 && qPoints[i].qHeight != 0)
			num += 1;
	}
	
	if(num < numPoints)
		numPoints = num;
					
	for(i = 0; i < numPoints; i++)
	{
		wholeStr += sprintf(wholeStr, "%d", qPoints[i].qWidth);
		wholeStr += sprintf(wholeStr, ",%d\n", qPoints[i].qHeight);
	}

	count = strlen(wholebuf);

	if (copy_to_user(buf, wholebuf, strlen(wholebuf)))
	{
		printk("EFAULT\n");		
		return -EFAULT;
	}

	numPoints = 0;
	return count;
}

static ssize_t mysnake_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	int temp;
	char tbuf[1024], *tbptr = tbuf;
	/* end of buffer reached */
	if (*f_pos >= capacity)
	{
		printk(KERN_INFO
			"write called: process id %d, command %s, count %d, buffer full\n",
			current->pid, current->comm, count);
		return -ENOSPC;
	}

	/* do not eat more than a bite */
	if (count > bite) count = bite;

	/* do not go over the end */
	if (count > capacity - *f_pos)
		count = capacity - *f_pos;

	if (copy_from_user(mysnake_buffer + *f_pos, buf, count))
	{
		return -EFAULT;
	}

	tbptr += sprintf(tbptr,								   
		"write called: process id %d, command %s, count %d, chars ",
		current->pid, current->comm, count);

	for (temp = *f_pos; temp < count + *f_pos; temp++)					  
		tbptr += sprintf(tbptr, "%c", mysnake_buffer[temp]);

	printk(KERN_INFO "%s\n", tbuf);

	*f_pos += count;
	mysnake_len = *f_pos;

	return count;
}

//snaketimer callback function when timer expires
static void snaketimer_callback(unsigned long data){
	int p = 0;
	int q = 0;
	m = 1; //starts at 1 to reserve 0 for "food"
	ptNum = 1; //starts at 1 to reserve 0 for "food"

	//GPIO Test
	if (b0_pressed == 1)
	{
		gesture_direction = 1;
	}
	if (b1_pressed == 1)
	{
		gesture_direction = 2;
	}
	if (b2_pressed == 1)
	{
		gesture_direction = 3;
	}
	if (b3_pressed == 1)
	{
		gesture_direction = 4;
	}
	
	//get gesture direction
	if (gesture_direction == 0)//nothing
	{
		//does nothing, waiting for gesture
	}
	else if (gesture_direction == 1)//down
	{
		switch (GUI_Array[pos_snakehead[0]+1][pos_snakehead[1]])
		{
			case 0:
				pos_snakehead[0] = pos_snakehead[0]+1;
				GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
				GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 0;
				//change snaketail position
				if (GUI_Array[pos_snaketail[0]+1][pos_snaketail[1]] == 1)
				{
					pos_snaketail[0]=pos_snaketail[0]+1;
				}
				else if(GUI_Array[pos_snaketail[0]-1][pos_snaketail[1]] == 1)
				{
					pos_snaketail[0]=pos_snaketail[0]-1;
				}
				else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]+1] == 1)
				{
					pos_snaketail[1]=pos_snaketail[1]+1;
				}
				else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]-1] == 1)
				{
					pos_snaketail[1]=pos_snaketail[1]-1;
				}
				else
				{
					printk("You don' messed up 1");
				}
				break;
			case 1:
				end_function();
				break;
			case 2:
				pos_snakehead[0] = pos_snakehead[0]+1;
				GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
				pos_randpoint[0] = randpoint_finder(width);
				pos_randpoint[1] = randpoint_finder(height);
				//printk("Wrong Vals");
				while (GUI_Array[pos_randpoint[0]][pos_randpoint[1]] != 0)
				{
					pos_randpoint[0] = randpoint_finder(width);
					pos_randpoint[1] = randpoint_finder(height);			
				}
				GUI_Array[pos_randpoint[0]][pos_randpoint[1]] = 2;
				break;
			case 3:
				end_function();
				break;
		}
		/*if (GUI_Array[pos_snakehead[0]+1][pos_snakehead[1]]==0)
		{
			pos_snakehead[0] = pos_snakehead[0]+1;
			GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
			GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 0;
			
			//change snaketail position
			if (GUI_Array[pos_snaketail[0]+1][pos_snaketail[1]] == 1)
			{
				pos_snaketail[0]=pos_snaketail[0]+1;
				
			}
			else if(GUI_Array[pos_snaketail[0]-1][pos_snaketail[1]] == 1)
			{
				pos_snaketail[0]=pos_snaketail[0]-1;
			}
			else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]+1] == 1)
			{
				pos_snaketail[1]=pos_snaketail[1]+1;
			}
			else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]-1] == 1)
			{
				pos_snaketail[1]=pos_snaketail[1]-1;
			}
			else
			{
				printk("You don' messed up 1");
			}
			
			//update GUI
			//GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 1;
				
		}
		else if (GUI_Array[pos_snakehead[0]+1][pos_snakehead[1]]==1 || GUI_Array[pos_snakehead[0]+1][pos_snakehead[1]]==3)
		{
			end_function();
		}
		else if (GUI_Array[pos_snakehead[0]+1][pos_snakehead[1]]==2)
		{
			pos_snakehead[0] = pos_snakehead[0]+1;
			GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
			pos_randpoint[0] = randpoint_finder(width);
			pos_randpoint[1] = randpoint_finder(height);
			//printk("Wrong Vals");
			while (GUI_Array[pos_randpoint[0]][pos_randpoint[1]] != 0)
			{
				pos_randpoint[0] = randpoint_finder(width);
				pos_randpoint[1] = randpoint_finder(height);			
			}
			GUI_Array[pos_randpoint[0]][pos_randpoint[1]] = 2;
		}
		else
		{
			printk("What went wrong = 1");
		}*/
	}
	else if (gesture_direction == 2) //up
	{
		switch (GUI_Array[pos_snakehead[0]-1][pos_snakehead[1]])
		{
			case 0:
				pos_snakehead[0] = pos_snakehead[0]-1;
				GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
				GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 0;
				//change snaketail position
				if (GUI_Array[pos_snaketail[0]+1][pos_snaketail[1]] == 1)
				{
					pos_snaketail[0]=pos_snaketail[0]+1;
				}
				else if(GUI_Array[pos_snaketail[0]-1][pos_snaketail[1]] == 1)
				{
					pos_snaketail[0]=pos_snaketail[0]-1;
				}
				else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]+1] == 1)
				{
					pos_snaketail[1]=pos_snaketail[1]+1;
				}
				else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]-1] == 1)
				{
					pos_snaketail[1]=pos_snaketail[1]-1;
				}
				else
				{
					printk("You don' messed up 2");
				}
				break;
			case 1:
				end_function();
				break;
			case 2:
				pos_snakehead[0] = pos_snakehead[0]-1;
				GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
				pos_randpoint[0] = randpoint_finder(width);
				pos_randpoint[1] = randpoint_finder(height);
				//printk("Wrong Vals");
				while (GUI_Array[pos_randpoint[0]][pos_randpoint[1]] != 0)
				{
					pos_randpoint[0] = randpoint_finder(width);
					pos_randpoint[1] = randpoint_finder(height);			
				}
				GUI_Array[pos_randpoint[0]][pos_randpoint[1]] = 2;
				break;
			case 3:
				end_function();
				break;
		}
		/*if (GUI_Array[pos_snakehead[0]-1][pos_snakehead[1]]==0)
		{
			pos_snakehead[0] = pos_snakehead[0]-1;
			GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
			GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 0;
			
			//change snaketail position
			if (GUI_Array[pos_snaketail[0]+1][pos_snaketail[1]] == 1)
			{
				pos_snaketail[0]=pos_snaketail[0]+1;
				
			}
			else if(GUI_Array[pos_snaketail[0]-1][pos_snaketail[1]] == 1)
			{
				pos_snaketail[0]=pos_snaketail[0]-1;
			}
			else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]+1] == 1)
			{
				pos_snaketail[1]=pos_snaketail[1]+1;
			}
			else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]-1] == 1)
			{
				pos_snaketail[1]=pos_snaketail[1]-1;
			}
			else
			{
				printk("You don' messed up 2");
			}
			
			//update GUI
			//GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 1;
	
		}
		else if (GUI_Array[pos_snakehead[0]-1][pos_snakehead[1]]==1 || GUI_Array[pos_snakehead[0]-1][pos_snakehead[1]]==3)
		{
			end_function();
		}
		else if (GUI_Array[pos_snakehead[0]-1][pos_snakehead[1]]==2)
		{
			pos_snakehead[0] = pos_snakehead[0]-1;
			GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
			pos_randpoint[0] = randpoint_finder(width);
			pos_randpoint[1] = randpoint_finder(height);
			//printk("Wrong Vals");
			while (GUI_Array[pos_randpoint[0]][pos_randpoint[1]] != 0)
			{
				pos_randpoint[0] = randpoint_finder(width);
				pos_randpoint[1] = randpoint_finder(height);			
			}
			GUI_Array[pos_randpoint[0]][pos_randpoint[1]] = 2;
		}
		else
		{
			printk("What went wrong = 2");
		}*/
	}	
	else if (gesture_direction == 3) //right
	{
		switch (GUI_Array[pos_snakehead[0]][pos_snakehead[1]+1])
		{
			case 0:
				pos_snakehead[1] = pos_snakehead[1]+1;
				GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
				GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 0;
				//change snaketail position
				if (GUI_Array[pos_snaketail[0]+1][pos_snaketail[1]] == 1)
				{
					pos_snaketail[0]=pos_snaketail[0]+1;
				}
				else if(GUI_Array[pos_snaketail[0]-1][pos_snaketail[1]] == 1)
				{
					pos_snaketail[0]=pos_snaketail[0]-1;
				}
				else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]+1] == 1)
				{
					pos_snaketail[1]=pos_snaketail[1]+1;
				}
				else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]-1] == 1)
				{
					pos_snaketail[1]=pos_snaketail[1]-1;
				}
				else
				{
					printk("You don' messed up 3");
				}
				break;
			case 1:
				end_function();
				break;
			case 2:
				pos_snakehead[1] = pos_snakehead[1]+1;
				GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
				pos_randpoint[0] = randpoint_finder(width);
				pos_randpoint[1] = randpoint_finder(height);
				//printk("Wrong Vals");
				while (GUI_Array[pos_randpoint[0]][pos_randpoint[1]] != 0)
				{
					pos_randpoint[0] = randpoint_finder(width);
					pos_randpoint[1] = randpoint_finder(height);			
				}
				GUI_Array[pos_randpoint[0]][pos_randpoint[1]] = 2;
				break;
			case 3:
				end_function();
				break;
		}
		/*if (GUI_Array[pos_snakehead[0]][pos_snakehead[1]+1]==0)
		{
			pos_snakehead[1] = pos_snakehead[1]+1;
			GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
			GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 0;
			
			//change snaketail position
			if (GUI_Array[pos_snaketail[0]+1][pos_snaketail[1]] == 1)
			{
				pos_snaketail[0]=pos_snaketail[0]+1;
				
			}
			else if(GUI_Array[pos_snaketail[0]-1][pos_snaketail[1]] == 1)
			{
				pos_snaketail[0]=pos_snaketail[0]-1;
			}
			else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]+1] == 1)
			{
				pos_snaketail[1]=pos_snaketail[1]+1;
			}
			else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]-1] == 1)
			{
				pos_snaketail[1]=pos_snaketail[1]-1;
			}
			else
			{
				printk("You don' messed up 3");
			}
			
			//update GUI
			//GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 1;
	
		}
		else if (GUI_Array[pos_snakehead[0]][pos_snakehead[1]+1]==1 || GUI_Array[pos_snakehead[0]][pos_snakehead[1]+1]==3)
		{
			end_function();
		}
		else if (GUI_Array[pos_snakehead[0]][pos_snakehead[1]+1]==2)
		{
			pos_snakehead[1] = pos_snakehead[1]+1;
			GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
			pos_randpoint[0] = randpoint_finder(width);
			pos_randpoint[1] = randpoint_finder(height);
			//printk("Wrong Vals");
			while (GUI_Array[pos_randpoint[0]][pos_randpoint[1]] != 0)
			{
				pos_randpoint[0] = randpoint_finder(width);
				pos_randpoint[1] = randpoint_finder(height);			
			}
			GUI_Array[pos_randpoint[0]][pos_randpoint[1]] = 2;
		}
		else
		{
			printk("What went wrong = 3");
		}*/
	}
	else if (gesture_direction == 4) //left
	{
		switch (GUI_Array[pos_snakehead[0]][pos_snakehead[1]-1])
		{
			case 0:
				pos_snakehead[1] = pos_snakehead[1]-1;
				GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
				GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 0;		
				//change snaketail position
				if (GUI_Array[pos_snaketail[0]+1][pos_snaketail[1]] == 1)
				{
					pos_snaketail[0]=pos_snaketail[0]+1;
				}
				else if(GUI_Array[pos_snaketail[0]-1][pos_snaketail[1]] == 1)
				{
					pos_snaketail[0]=pos_snaketail[0]-1;
				}
				else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]+1] == 1)
				{
					pos_snaketail[1]=pos_snaketail[1]+1;
				}
				else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]-1] == 1)
				{
					pos_snaketail[1]=pos_snaketail[1]-1;
				}
				else
				{
					printk("You don' messed up 4");
				}
				break;
			case 1:
				end_function();
				break;
			case 2:
				pos_snakehead[1] = pos_snakehead[1]-1;
				GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
				pos_randpoint[0] = randpoint_finder(width);
				pos_randpoint[1] = randpoint_finder(height);
				//printk("Wrong Vals");
				while (GUI_Array[pos_randpoint[0]][pos_randpoint[1]] != 0)
				{
					pos_randpoint[0] = randpoint_finder(width);
					pos_randpoint[1] = randpoint_finder(height);			
				}			
				GUI_Array[pos_randpoint[0]][pos_randpoint[1]] = 2;
				break;
			case 3:
				end_function();
				break;
		}
			
		/*if (GUI_Array[pos_snakehead[0]][pos_snakehead[1]-1]==0)
		{
			pos_snakehead[1] = pos_snakehead[1]-1;
			GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
			GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 0;
			
			//change snaketail position
			if (GUI_Array[pos_snaketail[0]+1][pos_snaketail[1]] == 1)
			{
				pos_snaketail[0]=pos_snaketail[0]+1;
				
			}
			else if(GUI_Array[pos_snaketail[0]-1][pos_snaketail[1]] == 1)
			{
				pos_snaketail[0]=pos_snaketail[0]-1;
			}
			else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]+1] == 1)
			{
				pos_snaketail[1]=pos_snaketail[1]+1;
			}
			else if(GUI_Array[pos_snaketail[0]][pos_snaketail[1]-1] == 1)
			{
				pos_snaketail[1]=pos_snaketail[1]-1;
			}
			else
			{
				printk("You don' messed up 4");
			}
			
			//update GUI
			//GUI_Array[pos_snaketail[0]][pos_snaketail[1]] = 1;
	
		}
		else if (GUI_Array[pos_snakehead[0]][pos_snakehead[1]-1]==1 || GUI_Array[pos_snakehead[0]][pos_snakehead[1]-1]==3)
		{
			end_function();
		}
		else if (GUI_Array[pos_snakehead[0]][pos_snakehead[1]-1]==2)
		{
			pos_snakehead[1] = pos_snakehead[1]-1;
			GUI_Array[pos_snakehead[0]][pos_snakehead[1]] = 1;
			pos_randpoint[0] = randpoint_finder(width);
			pos_randpoint[1] = randpoint_finder(height);
			//printk("Wrong Vals");
			while (GUI_Array[pos_randpoint[0]][pos_randpoint[1]] != 0)
			{
				pos_randpoint[0] = randpoint_finder(width);
				pos_randpoint[1] = randpoint_finder(height);			
			}			
			GUI_Array[pos_randpoint[0]][pos_randpoint[1]] = 2;
		}
		else
		{
			printk("What went wrong = 4");
		}*/
	}
	
	/*for (p = 0; p<width; p++)
	{
		for (q = 0; q<height; q++)
		{
			printk("%d", GUI_Array[p][q]);
		}
		printk("\n");
	}*/
	
	//start at 1 because index 0 is reserved for the "food" point
	for (p = 0; p <(width); p++)
	{
		for(q = 0; q< (height); q++)
		{
			if(GUI_Array[p][q] == 1)
			{
				qPoints[m].qWidth = p;
				qPoints[m].qHeight = q;
				m++; //increment index in list of points
				numPoints++;
			}
			if (GUI_Array[p][q] == 2)
			{			
				qPoints[0].qWidth = p;
				qPoints[0].qHeight = q;
				numPoints++;
			}
		}
	}
	/*for (m = 0; m < 7; m++)
	{
		printk("%d, %d\n", qPoints[m].qWidth, qPoints[m].qHeight);
	}
	printk("\n");*/

	/*if(gesture_direction == 1)
		printk("down\n");
	if(gesture_direction == 2)
		printk("up\n");
	if(gesture_direction == 3)
		printk("right\n");
	if(gesture_direction == 4)
		printk("left\n");*/
	
	//h = 0; //reset read function index
	mod_timer(snaketimer, jiffies + msecs_to_jiffies(period));
	return;
}

static int randpoint_finder(int max)
{
	int rand_num;
	
	rand_num = random32();
	
	while (rand_num > max-2)
	{
		rand_num = rand_num/2;
	}
	if (rand_num <= 0)
	{
		rand_num = 5;
	}


	return rand_num;
}

static void end_function(void)
{
	//printk("You Lost");
	start();
	
	return;
}

static void debounce_callback(unsigned long data)
{
	b0_pressed = 0;
	b1_pressed = 0;
	b2_pressed = 0;
	b3_pressed = 0;
	
	if (gpio_get_value(9))
	{
		b0_pressed = 1;
	}
	if (gpio_get_value(101))
	{
		b1_pressed = 1;
	}		
	if (gpio_get_value(28))
	{
		b2_pressed = 1;
	}	
	if (gpio_get_value(31))
	{
		b3_pressed = 1;
	}
	return;
}

static void start(void){

	h = 0;
		
	//flag = 1; //restart flag
		
	//test buttons
	gesture_direction = 0;
	b0_pressed = 0;
	b1_pressed = 0;
	b2_pressed = 0;
	b3_pressed = 0;
	

	//initialize GUI_Array to 0
	for (kk = 0; kk<height; kk++)
	{
		for (ll = 0; ll<width; ll++)
		{
			GUI_Array[ll][kk] = 0;
		}
	}
		
	//set border, initial snake position, and initial random point in array and position values
	for (ii = 0; ii<height; ii++)
	{
		GUI_Array[0][ii] = 3;
		GUI_Array[width-1][ii] = 3;
	}
	for (jj = 0; jj<width; jj++)
	{
		GUI_Array[jj][0] = 3;
		GUI_Array[jj][height-1] = 3;
	}
	pos_snakehead[0] = width/2 - 1;
	pos_snakehead[1] = height/2 - 1;
	pos_snaketail[0] = width/2 - 1;
	pos_snaketail[1] = height/2 - 1;
	snake_length = 1;

	GUI_Array[pos_snakehead[0]][pos_snakehead[1]]=1;
	pos_randpoint[0] = randpoint_finder(width-1);
	pos_randpoint[1] = randpoint_finder(height-1);
	GUI_Array[pos_randpoint[0]][pos_randpoint[1]]=2;
	

	//reset 1's array
	for (h = 0; h < 414; h++)
	{
		qPoints[h].qWidth = 0;
		qPoints[h].qHeight = 0;
		//m++; //increment index in list of points
				//numPoints++;
	}
			
	//qPoints[0].qWidth = 0;
	//qPoints[0].qHeight = 0;
				//numPoints++;
	//numPoints = 0;

	//printk("XXX");
	return;
}
