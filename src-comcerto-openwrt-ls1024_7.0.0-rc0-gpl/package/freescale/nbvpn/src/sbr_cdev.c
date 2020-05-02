/**************************************************************************
 * Copyright 2015, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        sbr_dev.c
 *
 * Description: Charracter driver for SBR project
 *
 * Authors:     Sridhar Pothuganti <sridhar.pothuganti@freescale.com>
 *
 */
/* History
 *  Version     Date            Author                  Change Description
 *    1.0       19/07/2015      Sridhar Pothuganti      Initial Development
 *    1.1       22/07/2015      Chaitanya Sakinam	Added ioctls for nbvpn
*/
/****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/version.h>

#include "sbr_ioctl.h"
#include "nbvpn_ctrl.h"

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int char_device_ioctl(struct file *filp,
			     unsigned int cmd, unsigned long arg);

#define SUCCESS 0
#define FAILURE -1

#define DEVICEDRV_NAME "sbr_dev"	/* Dev name as it appears in /proc/devices    */
#define DEVICE_NAME "sbr_dev"	/* Dev name as it appears in /proc/devices    */
#define DEVICE_CLASS_NAME "sbr_dev"	/* Dev name as it appears in /sys/    */

static int Device_Open = 0;	/* Is device open?
				 * Used to prevent multiple access to device */

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = char_device_ioctl
};

struct cdev *sbr_cdev;
dev_t dev;
static struct class *sms_class = NULL;	//udev support
static int char_device_id;

int sbr_init(void)
{
	int ret;
	ret = alloc_chrdev_region(&dev, 0, 1, DEVICEDRV_NAME);	/* allocating major number and no. of minor numbers (in dev) to my char driver name sc DEVICEDRV_NAME */

	sms_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME);

	if (IS_ERR(sms_class)) {
		printk(KERN_ERR "Error registering device class\n");
	}

	device_create(sms_class, NULL, dev, NULL, DEVICE_NAME);
	char_device_id = MAJOR(dev);	//extract major no

	/*Register our character Device */
	sbr_cdev = cdev_alloc();

	sbr_cdev->owner = THIS_MODULE;
	sbr_cdev->ops = &fops;

	ret = cdev_add(sbr_cdev, dev, 1);
	if (ret < 0) {
		printk("Error registering device driver\n");
		return ret;
	}
	printk("Device Registered with MAJOR NO[%d]\n", char_device_id);

	return SUCCESS;
}

void sbr_exit(void)
{
	device_destroy(sms_class, dev);
	class_destroy(sms_class);
	unregister_chrdev_region(dev, 1);
	cdev_del(sbr_cdev);
	printk("Character Device Un Registered \n");
}

static int device_open(struct inode *inode, struct file *file)
{
	if (Device_Open)
		return EBUSY;
	Device_Open = 1;	/* We're now ready for our next caller */
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	Device_Open = 0;	/* We're now ready for our next caller */
/*
 * Decrement the usage count, or else once you opened the file, you'll
 * never get get rid of the module.
 */
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{

	printk(KERN_ALERT "Sorry, this Read operation isn't supported.\n");
	return length;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
static ssize_t device_write(struct file *filp, const char *buff, size_t len,
			    loff_t * off)
{
	printk(KERN_ALERT "Sorry, this Write operation isn't supported.\n");
	return len;
}

static int char_device_ioctl(struct file *filp,
			     unsigned int cmd, unsigned long arg)
{

	switch (cmd) {
	case SBR_ADD_NBVPN_REC:
		sbr_nbvpn_ioctl_add((nbvpn_ctrl_t *) arg);
		break;

	case SBR_DEL_NBVPN_REC:
		sbr_nbvpn_ioctl_del((nbvpn_ctrl_t *) arg);
		break;

	case SBR_GET_FIRST_NBVPN_REC:
		sbr_nbvpn_ioctl_get_first((nbvpn_ctrl_t *) arg);
		break;

	case SBR_GET_NEXT_NBVPN_REC:
		sbr_nbvpn_ioctl_get_next((nbvpn_ctrl_t *) arg);
		break;

	case SBR_LIST_NBVPN_REC:
		sbr_nbvpn_ioctl_list();
		break;

	default:
		return FAILURE;
	}
	return SUCCESS;
}

module_init(sbr_init);
module_exit(sbr_exit);

MODULE_DESCRIPTION("Char Driver for add/remove records using IOCTL");
MODULE_AUTHOR("sridhar.pothuganti@freescale.com");
MODULE_LICENSE("GPL");
