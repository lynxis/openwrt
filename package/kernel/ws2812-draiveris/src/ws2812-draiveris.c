/*
 *  ws2812-draiveris.c - A bitbang kernel module for ws2812
 *
 *  Copyright (C) 2006-2012 OpenWrt.org
 *  Copyright (C) 2012 Žilvinas Valinskas, Saulius Lukšė
 *  Copyright (C) 2014 Jürgen Weigert <jw@owncloud.com>
 *  Copyright (C) 2016 Lukas Zeller <luz@plan44.ch> (RGBW support)
 *  Copyright (C) 2021 Alexander Couzens <lynxis@fe80.eu>
 *
 *  This is free software, licensed under the GNU General Public License v2.
 *  See /LICENSE for more information.
 *
 * parameters:
 *	gpio_number=20		# set base number.
 *	gpio_count=3		# default: 1, enable multiple outputs, counting from gpio_number.
 *	gpios=20,21,22		# explicitly specify where the led_strips are connected.
 *	leds_per_gpio		# length of the led strips.
 *	inverted=0		# have inverting line drivers at the GPIOs
 *	rgbw=0			# set 1 for quad-channel RGBW LEDs (SK6812)
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>

#include <linux/types.h>
#include <linux/string.h>

#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/of.h>

#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/watchdog.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/fs.h>


#define WS2812_MAJOR		152	/* use a LOCAL/EXPERIMENTAL major for now */
/* TODO: rework module parameters */
#define LED_STRIPS_SEQUENTIAL	1	/* 0: PARALLEL: one set of values for all outputs */
#define GPIO_NUMBER_DEFAULT 11

static int gpio_number = -1; // default -1 => GPIO_NUMBER_DEFAULT
module_param(gpio_number, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(gpio_number, "GPIO number of first chain. Either use 'gpio_number=FIRST gpio_count=COUNT' or use 'gpios=FIRST,SECOND,THIRD,...'.");

static int inverted = 0; // default is 0 == normal. 1 == inverted is good for 74HCT02 line drivers.
module_param(inverted, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(inverted, "drive inverted outputs");

static int rgbw = 0; // default is 0 == WS21812. 1 == SK6812 RGBW LEDs
module_param(rgbw, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(rgbw, "RGBW LEDs (SK6812)");

static int gpio_count = 1; // default is 3 == the board, I currently have.
module_param(gpio_count, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(gpio_count, "use additional GPIOs if > 1.");

static int leds_per_gpio = 24; // only used when gpio_count > 1.
module_param(leds_per_gpio, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(leds_per_gpio, "start of next led chain");

static char *gpios = NULL;
module_param(gpios, charp, 0);
MODULE_PARM_DESC(gpios, "Comma separated list of GPIO numbers. Either use this, or 'gpio_number= gpio_count='.");

#define DEVICE_NAME "ws2812"
static struct class *ws2812_class;

/* TODO: add skw support */
struct ws2812 {
	struct work_struct work;
	struct cdev cdev;
	struct gpio_desc *gpiod;
	uint32_t leds;
	char *frame_buf;
	size_t frame_len;
};

static inline void encodezero(struct ws2812 *priv) {
	/* 0.35us high (+-150ns), 0.8us low (+-150ns) */
	gpiod_set_value(priv->gpiod, 1);
	ndelay(200);
	gpiod_set_value(priv->gpiod, 0);
	ndelay(550);
}

static inline void encodeone(struct ws2812 *priv) {
	/* 0.7us high (+-150ns), 0.6us low (+-150ns) */
	gpiod_set_value(priv->gpiod, 1);
	ndelay(650);
	gpiod_set_value(priv->gpiod, 0);
	ndelay(450);
}

static void ws2812_update_handler(struct work_struct *work)
{
	int i, j;
	unsigned long flags;
	DEFINE_SPINLOCK(critical);
	struct ws2812 *priv = container_of(work, struct ws2812, work);

	gpiod_set_value(priv->gpiod, 0);
	usleep_range(60, 100);
	/* reset for >= 50 us */
	spin_lock_irqsave(&critical, flags);
	for (i = 0; i < priv->frame_len; i++) {
		uint8_t col = priv->frame_buf[i];
		for (j = 0; j < 8; j++) {
			if (col & 0x80)
				encodeone(priv);
			else
				encodezero(priv);
			col <<= 1;
		}
	}
	gpiod_set_value(priv->gpiod, 0);
	spin_unlock_irqrestore(&critical, flags);
	/* reset for >= 50 us */
	usleep_range(60, 100);
}

static int ws2812_open(struct inode *inode, struct file *file)
{
	struct ws2812 *priv;

	priv = container_of(inode->i_cdev, struct ws2812, cdev);
	file->private_data = priv;

	return nonseekable_open(inode, file);
}

static ssize_t ws2812_read(struct file *file, char *user_buf, size_t len, loff_t *off)
{
	struct ws2812 *priv = file->private_data;
	return simple_read_from_buffer(user_buf, len, off, priv->frame_buf, priv->frame_len);
}

static ssize_t ws2812_write(struct file *file, const char *buff, size_t len, loff_t *off)
{
	struct ws2812 *priv = file->private_data;
	int leds, led;

	leds = len / 3;
	if (!leds)
		return len;

	if (leds > priv->leds)
		leds = priv->leds;

	for (led = 0; led < leds; led++) {
		unsigned int off = led * 3;
		/* frame_buf contains the value prepared for the wire. green, red, blue */
		priv->frame_buf[off + 1] = buff[off + 0]; /* red */
		priv->frame_buf[off + 0] = buff[off + 1]; /* green */
		priv->frame_buf[off + 2] = buff[off + 2]; /* blue */
	}
	schedule_work(&priv->work);
	return len;
}

static struct file_operations ws2812_fops = {
	.owner = THIS_MODULE,
	.read = ws2812_read,
	.write = ws2812_write,
	.open = ws2812_open,
	.llseek = no_llseek
};


static int ws2812_probe(struct platform_device *pdev)
{
	struct ws2812 *priv;
	int err;

	/* TODO: use platform data and module args */
	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	INIT_WORK(&priv->work, &ws2812_update_handler);

	err = of_property_read_u32(pdev->dev.of_node, "ws2812,leds", &priv->leds);
	if (err < 0) {
		dev_err(&pdev->dev, "Missing of ws2812,leds property\n");
		return err;
	}

	priv->frame_len = priv->leds * 3;
	priv->frame_buf = devm_kzalloc(&pdev->dev, priv->frame_len, GFP_KERNEL);
	if (!priv->frame_buf)
		return -ENOMEM;

	priv->gpiod = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
	if (IS_ERR(priv->gpiod)) {
		dev_err(&pdev->dev, "Failed to get the gpio\n");
		return PTR_ERR(priv->gpiod);
	}

	platform_set_drvdata(pdev, priv);
	cdev_init(&priv->cdev, &ws2812_fops);
	cdev_add(&priv->cdev, MKDEV(WS2812_MAJOR, 0), 1);

	dev_info(&pdev->dev, "registered ws2812 via 111 gpio %d\n", desc_to_gpio(priv->gpiod));
	return 0;
}

static const struct of_device_id of_ws2812_gpio_match[] = {
		{ .compatible = "ws2812-gpio", },
		{},
};
MODULE_DEVICE_TABLE(of, of_ws2812_gpio_match);

static struct platform_driver ws2812_gpio_driver = {
		.probe		  = ws2812_probe,
		.driver		 = {
				.name   = "ws2812-gpio",
				.of_match_table = of_ws2812_gpio_match,
		},
};

static int ws2812_init(void)
{
	int err;
	ws2812_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(ws2812_class)) {
		pr_err("Failed to create ws2812 class\n");
		return PTR_ERR(ws2812_class);
	}

	err = register_chrdev_region(MKDEV(WS2812_MAJOR, 0), 16, DEVICE_NAME);
	if (err < 0) {
		err = WS2812_MAJOR;
		pr_warn("Failed to register ws2812 char device. Err %d\n", err);
		goto out_class;
	}

	err = platform_driver_register(&ws2812_gpio_driver);
	if (err < 0)
		goto out_chrdev;

	return 0;

out_chrdev:
	unregister_chrdev(WS2812_MAJOR, DEVICE_NAME);
out_class:
	class_destroy(ws2812_class);
	return err;
}

void ws2812_cleanup(void)
{
	device_destroy(ws2812_class, MKDEV(WS2812_MAJOR, 0));
	class_destroy(ws2812_class);
}

module_init(ws2812_init);
module_exit(ws2812_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Couzens <lynxis@fe80.eu>");
MODULE_DESCRIPTION("Bitbang GPIO driver for WS2812 led chains");
MODULE_ALIAS_CHARDEV_MAJOR(WS2812_MAJOR);
