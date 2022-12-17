/*
 * al_dec.c userspace interface and probe / remove of the driver
 *
 * Copyright (C) 2019, Allegro DVT (www.allegrodvt.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <linux/io.h>
#include <linux/uaccess.h>

#include <linux/cdev.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/fcntl.h>
#include <linux/firmware.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/vmalloc.h>
#include <linux/version.h>

#include "al_dec_ioctl.h"
#include "al_alloc_ioctl.h"
#include "al_vcu.h"
#include "al_alloc.h"
#include "al_user.h"
#include "al_codec.h"
#include "dec_user.h"
#include "al_char.h"

#include "mcu_interface.h"
#include "al_mail.h"
#include "dec_mails_factory.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
#include "xil_clk.h"
#endif

#define AL5D_FIRMWARE "al5d.fw"
#define AL5D_BOOTLOADER_FIRMWARE "al5d_b.fw"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
#define AL5D_CORE_CLK "vcu_core_dec"
#define AL5D_MCU_CLK "vcu_mcu_dec"
#endif

static int max_users_nb = MAX_USERS_NB;
static int al5d_codec_major;
static int al5d_codec_nr_devs = AL5_NR_DEVS;
static struct class *module_class;

static int ioctl_usage(struct al5_user *user, unsigned int cmd)
{
	pr_err("Unknown ioctl: 0x%.8X\n", cmd);
	pr_err("Known ioctl are:\n");
	pr_err("AL_MCU_CONFIG_CHANNEL:%.8lX\n",
	       (unsigned long)AL_MCU_CONFIG_CHANNEL);
	pr_err("AL_MCU_DESTROY_CHANNEL:%.8lX\n",
	       (unsigned long)AL_MCU_DESTROY_CHANNEL);
	pr_err("AL_MCU_DECODE_ONE_FRM:%.8lX\n",
	       (unsigned long)AL_MCU_DECODE_ONE_FRM);
	pr_err("AL_MCU_WAIT_FOR_STATUS:%.8lX\n",
	       (unsigned long)AL_MCU_WAIT_FOR_STATUS);
	pr_err("AL_MCU_SEARCH_START_CODE:%.8lX\n",
	       (unsigned long)AL_MCU_SEARCH_START_CODE);
	pr_err("GET_DMA_FD:%.8lX\n",
	       (unsigned long)GET_DMA_FD);

	return -EINVAL;
}

static long al5d_ioctl(struct file *filp, unsigned int cmd,
		       unsigned long arg)
{
	struct al5_filp_data *filp_data = filp->private_data;
	struct al5_user *user = filp_data->user;
	struct al5_codec_desc *codec = filp_data->codec;
	long ret;

	switch (cmd) {
		struct al5_channel_config channel_config;
		struct al5_params params;
		struct al5_decode_msg decode_msg;
		struct al5_search_sc_msg sc_msg;
		struct al5_scstatus sc_status;
	case AL_MCU_CONFIG_CHANNEL:
		ioctl_info("ioctl AL_MCU_CONFIG_CHANNEL from user %i",
			   user->uid);
		if (copy_from_user(&channel_config, (void *)arg,
				   sizeof(channel_config)))
			return -EFAULT;
		ret = al5d_user_create_channel(user, &channel_config);
		if (copy_to_user((void *)arg, &channel_config,
				 sizeof(channel_config)))
			return -EFAULT;
		ioctl_info("end AL_MCU_CONFIG_CHANNEL for user %i", user->uid);
		return ret;

	case AL_MCU_DESTROY_CHANNEL:
		ioctl_info("ioctl AL_MCU_DESTROY_CHANNEL from user %i",
			   user->uid);
		ret = al5_user_destroy_channel(user, false);
		ioctl_info("end AL_MCU_DESTROY_CHANNEL for user %i", user->uid);
		return ret;

	case AL_MCU_WAIT_FOR_STATUS:
		ioctl_info("ioctl AL_MCU_WAIT_FOR_STATUS from user %i",
			   user->uid);
		if (copy_from_user(&params, (void *)arg, sizeof(params)))
			return -EFAULT;
		ret = al5d_user_wait_for_status(user, &params);
		if (copy_to_user((void *)arg, &params, sizeof(params)))
			return -EFAULT;
		ioctl_info("end AL_MCU_WAIT_FOR_STATUS for user %i", user->uid);
		return ret;

	case AL_MCU_DECODE_ONE_FRM:
		ioctl_info("ioctl AL_MCU_DECODE_ONE_FRM from user %i",
			   user->uid);
		if (copy_from_user(&decode_msg, (void *)arg,
				   sizeof(decode_msg)))
			return -EFAULT;
		ret = al5d_user_decode_one_frame(user, &decode_msg);
		if (copy_to_user((void *)arg, &decode_msg, sizeof(decode_msg)))
			return -EFAULT;
		ioctl_info("end AL_MCU_DECODE_ONE_FRM for user %i", user->uid);
		return ret;

	case AL_MCU_DECODE_ONE_SLICE:
		ioctl_info("ioctl AL_MCU_DECODE_ONE_SLICE from user %i",
			   user->uid);
		if (copy_from_user(&decode_msg, (void *)arg,
				   sizeof(decode_msg)))
			return -EFAULT;
		ret = al5d_user_decode_one_slice(user, &decode_msg);
		if (copy_to_user((void *)arg, &decode_msg, sizeof(decode_msg)))
			return -EFAULT;
		ioctl_info("end AL_MCU_DECODE_ONE_SLICE for user %i",
			   user->uid);
		return ret;

	case AL_MCU_SEARCH_START_CODE:
		ioctl_info("ioctl AL_MCU_SEARCH_START_CODE from user %i",
			   user->uid);
		if (copy_from_user(&sc_msg, (void *)arg, sizeof(sc_msg)))
			return -EFAULT;
		ret = al5d_user_search_start_code(user, &sc_msg);
		if (copy_to_user((void *)arg, &sc_msg, sizeof(sc_msg)))
			return -EFAULT;
		ioctl_info("end AL_MCU_SEARCH_START_CODE for user %i",
			   user->uid);
		return ret;

	case AL_MCU_WAIT_FOR_START_CODE:
		ioctl_info("ioctl AL_MCU_WAIT_FOR_START_CODE from user %i",
			   user->uid);
		if (copy_from_user(&sc_status, (void *)arg, sizeof(sc_status)))
			return -EFAULT;
		ret = al5d_user_wait_for_start_code(user, &sc_status);
		if (copy_to_user((void *)arg, &sc_status, sizeof(sc_status)))
			return -EFAULT;
		ioctl_info("end AL_MCU_WAIT_FOR_START_CODE for user %i",
			   user->uid);
		return ret;

	case GET_DMA_FD:
		ret = al5_ioctl_get_dma_fd(codec->device, arg);
		return ret;

	case GET_DMA_PHY:
		ret = al5_ioctl_get_dmabuf_dma_addr(codec->device, arg);
		return ret;

	default:
		return ioctl_usage(user, cmd);
	}

	return 0;
}

static const struct file_operations al5d_fops = {
	.owner		= THIS_MODULE,
	.open		= al5_codec_open,
	.release	= al5_codec_release,
	.unlocked_ioctl = al5d_ioctl,
	.compat_ioctl	= al5_codec_compat_ioctl,
	.poll		= al5_codec_poll,
};

static int al5d_setup_codec_cdev(struct al5_codec_desc *codec, int minor)
{
	struct device *device;
	const char *device_name = "allegroDecodeIP";
	dev_t dev = MKDEV(al5d_codec_major, minor);

	int err = al5_setup_codec_cdev(codec, &al5d_fops, THIS_MODULE,
				       al5d_codec_major,
				       minor);

	if (err)
		return err;

	of_property_read_string(codec->device->of_node, "al,devicename", &device_name);

	device = device_create(module_class, NULL, dev, NULL, device_name);
	if (IS_ERR(device)) {
		pr_err("device not created\n");
		al5_clean_up_codec_cdev(codec);
		return PTR_ERR(device);
	}

	return 0;
}

static int al5d_codec_probe(struct platform_device *pdev)
{
	int err;
	static int current_minor;

	struct al5_codec_desc *codec;

	codec = devm_kzalloc(&pdev->dev,
			     sizeof(struct al5_codec_desc),
			     GFP_KERNEL);
	if (codec == NULL)
		return -ENOMEM;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
	err = xil_clk_set_up(pdev, codec, AL5D_CORE_CLK, AL5D_MCU_CLK);
	if (err) {
		dev_err(&pdev->dev, "Failed to setup clock");
		return err;
	}
#endif

	err = al5_codec_set_up(codec, pdev, max_users_nb);
	if (err) {
		dev_err(&pdev->dev, "Failed to setup codec");
		return err;
	}
	err = al5_codec_set_firmware(codec, AL5D_FIRMWARE,
				     AL5D_BOOTLOADER_FIRMWARE);
	if (err) {
		dev_err(&pdev->dev, "Failed to setup firmware");
		al5_codec_tear_down(codec);
		return err;
	}
	err = al5d_setup_codec_cdev(codec, current_minor);
	if (err) {
		dev_err(&pdev->dev, "Failed to setup cdev");
		al5_codec_tear_down(codec);
		return err;
	}
	codec->minor = current_minor;
	++current_minor;

	return 0;
}

static int al5d_codec_remove(struct platform_device *pdev)
{
	struct al5_codec_desc *codec = platform_get_drvdata(pdev);
	dev_t dev = MKDEV(al5d_codec_major, codec->minor);

	al5_codec_tear_down(codec);
	device_destroy(module_class, dev);
	al5_clean_up_codec_cdev(codec);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
	xil_clk_clean_up(pdev, codec);
#endif
	return 0;
}

static const struct of_device_id al5d_codec_of_match[] = {
	{ .compatible = "al,al5d" },
	{ /* sentinel */ },
};

MODULE_DEVICE_TABLE(of, al5d_codec_of_match);

static struct platform_driver al5d_platform_driver = {
	.probe			= al5d_codec_probe,
	.remove			= al5d_codec_remove,
	.driver			=       {
		.name		= "al5d",
		.of_match_table = of_match_ptr(al5d_codec_of_match),
	},
};

static int setup_chrdev_region(void)
{
	return al5_setup_chrdev_region(&al5d_codec_major, 0,
				       al5d_codec_nr_devs, "al5d");
}

static int create_module_class(void)
{
	module_class = class_create(THIS_MODULE, "allegro_decode_class");
	if (IS_ERR(module_class))
		return PTR_ERR(module_class);

	return 0;
}

static void destroy_module_class(void)
{
	class_destroy(module_class);
}

static int __init al5d_codec_init(void)
{
	int err;

	err = setup_chrdev_region();
	if (err)
		return err;

	err = create_module_class();
	if (err)
		return err;

	return platform_driver_register(&al5d_platform_driver);
}

static void __exit al5d_codec_exit(void)
{
	dev_t devno = MKDEV(al5d_codec_major, 0);

	platform_driver_unregister(&al5d_platform_driver);
	destroy_module_class();
	unregister_chrdev_region(devno, al5d_codec_nr_devs);
}

module_init(al5d_codec_init);
module_exit(al5d_codec_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Kevin Grandemange");
MODULE_AUTHOR("Sebastien Alaiwan");
MODULE_AUTHOR("Antoine Gruzelle");
MODULE_DESCRIPTION("Allegro Codec Driver");
