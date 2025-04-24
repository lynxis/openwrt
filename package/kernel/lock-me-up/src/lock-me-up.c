/*
 *  Lock Me Up
 *
 *  Copyright (C) 2025 Alexander Couzens <lynxis@fe80.eu>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kmod.h>

#include <linux/workqueue.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/kobject.h>
#include <linux/delay.h>

#define DRV_NAME	"lock-me-up"
#define DRV_VERSION	"0.1.1"
#define DRV_DESC	"Good lock! Cheap and nice."


static struct kobject *lock_me_up_kobject;

static struct work_struct	*lock_me_up_work_ptr;
static volatile int lock_me_up_flag = 0;


static ssize_t lock_me_up_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk(KERN_ERR DRV_NAME " Schedule locking up in 5 sec");

	lock_me_up_flag = 1;
	schedule_work(lock_me_up_work_ptr);

	return sprintf(buf, "Schedule locking in 5 sec\n");
}

static ssize_t lock_me_up_set(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	printk(KERN_ERR DRV_NAME " Freeing lock again");

	lock_me_up_flag = 0;

	return count;
}

static struct kobj_attribute lock_me_up_attribute = __ATTR(lockmeup, 0600, lock_me_up_show, lock_me_up_set);

static void lock_me_up_work(struct work_struct *work)
{
	/* FIXME: lock here with irq */
	printk(KERN_ERR DRV_NAME " Locking me up...");
	unsigned long flags;
	spinlock_t lock;
	spin_lock_irqsave(&lock, flags);
	while (lock_me_up_flag) {
		udelay(100);
	}
	spin_unlock_irqrestore(&lock, flags);
}

/* -------------------------------------------------------------------------*/

static int __init lock_me_up_init(void)
{
	int ret = 0;

	lock_me_up_kobject = kobject_create_and_add("lock_me_up", kernel_kobj);
	if (!lock_me_up_kobject) {
		pr_err(DRV_NAME " failed to create the kobject\n");
		return -ENOMEM;
	}

	ret = sysfs_create_file(lock_me_up_kobject, &lock_me_up_attribute.attr);
	if (ret) {
		pr_err(DRV_NAME " failed to create the sysfs\n");
	}

	lock_me_up_work_ptr = kzalloc(sizeof(struct work_struct), GFP_KERNEL);
	INIT_WORK(lock_me_up_work_ptr, (void *)(void *)lock_me_up_work);

	printk(KERN_INFO DRV_DESC " version " DRV_VERSION "\n");
	return ret;
}
module_init(lock_me_up_init);

static void __exit lock_me_up_exit(void)
{
	kobject_put(lock_me_up_kobject);
}
module_exit(lock_me_up_exit);

MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("Alexander Couzens <lynxis@fe80.eu>");
MODULE_LICENSE("GPL v2");

