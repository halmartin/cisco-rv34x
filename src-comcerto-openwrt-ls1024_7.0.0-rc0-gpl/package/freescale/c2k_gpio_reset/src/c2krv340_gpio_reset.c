/* Module to reset target device through GPIO04 reset pin */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 
#include <linux/interrupt.h>            
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kthread.h>

static struct task_struct *reset_thread;

static const char * const reboot_argv[] = 
    { "/sbin/reboot", NULL };

static const char * const factory_reset_argv[] =
    { "/usr/sbin/config_mgmt.sh", "factory-default" , NULL };

static const char * const cert_reset_argv[] =
    { "/usr/bin/delete_certificates", "factory_default" , NULL };


static int c2krv340_gpio_reboot(void)
{
	printk(KERN_INFO "%s \n",__func__);
	call_usermodehelper(reboot_argv[0], reboot_argv, NULL, UMH_NO_WAIT);
}

static int c2krv340_gpio_factory_reset(void)
{
        printk(KERN_INFO "%s \n",__func__);
        call_usermodehelper(cert_reset_argv[0], cert_reset_argv, NULL, UMH_WAIT_PROC);
        call_usermodehelper(factory_reset_argv[0], factory_reset_argv, NULL, UMH_WAIT_PROC);
}

static int c2krv340_gpio_get_value(int offset)
{
	if (offset < 32)
		return ((__raw_readl(COMCERTO_GPIO_INPUT_REG) >> offset) & 0x1) ? 1 : 0 ;
	else if (offset < 64)
		return (( __raw_readl(COMCERTO_GPIO_INPUT_REG) >> (offset - 32)) & 0x1) ? 1 : 0;
	else
		return -EINVAL;
}

int c2krv340_reset_thread(void) 
{
	int p1=0;

	printk(KERN_INFO "%s: In thread\n",__func__);

	while(1)
   	{
        	if(c2krv340_gpio_get_value(GPIO_PIN_NUM_4) == 0)
                	p1++;
		
		msleep(100);

        	if(((p1 >= 1) && (p1 < 100)) && (c2krv340_gpio_get_value(GPIO_PIN_NUM_4)))
        	{
                	printk(KERN_INFO "%s: *******  SYSTEM REBOOT ******\n",__func__);
                	p1 = 0;
                	c2krv340_gpio_reboot();
        	}
        	else if((p1 >= 100) && (c2krv340_gpio_get_value(GPIO_PIN_NUM_4)))
        	{
                	printk(KERN_INFO "%s: *******  FACTORY RESET ******\n",__func__);
                	p1 = 0;
			c2krv340_gpio_factory_reset();
			c2krv340_gpio_reboot();
        	}

  	}

	return 0;
}
 
static int __init c2krv340_gpio_reset_init(void)
{
	printk(KERN_INFO "%s:  initializing GPIO pin: %d\n",__func__,GPIO_PIN_NUM_4);

	/* select GPIO04 pin mux */
	__raw_writel(__raw_readl(COMCERTO_GPIO_PIN_SELECT_REG) & ~(0x3 << 8), COMCERTO_GPIO_PIN_SELECT_REG);

	/* configure GPIO04 as input */
	__raw_writel(__raw_readl(COMCERTO_GPIO_OE_REG) & ~(0x1 << 4), COMCERTO_GPIO_OE_REG);

	/* creating kernel thread to monitor GPIO04 status */
	reset_thread = kthread_create(c2krv340_reset_thread,NULL,"c2krv340_reset");
	if(reset_thread)
	{
		printk(KERN_INFO "%s: thread created",__func__);
		wake_up_process(reset_thread);
	}

	printk(KERN_INFO "%s:	return success\n",__func__);
	return 0;
}

static void __exit c2krv340_gpio_reset_exit(void)
{
	printk(KERN_INFO "%s  \n",__func__);
}

module_init(c2krv340_gpio_reset_init);
module_exit(c2krv340_gpio_reset_exit);
MODULE_LICENSE("GPL");
