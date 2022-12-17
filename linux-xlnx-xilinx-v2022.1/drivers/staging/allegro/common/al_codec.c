/*
 * al_codec.c per device operation. functions to change the state of the vcu
 * and functions to handle channels created by the user.
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

#include <linux/delay.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/of.h>
#include <linux/version.h>

#include "al_mail.h"
#include "al_codec_mails.h"
#include "mcu_utils.h"
#include "al_codec.h"
#include "al_alloc.h"
#include "mcu_interface.h"

#ifdef CONFIG_MEMORY_HOTPLUG
#define HOTPLUG_ALIGN 0x40000000
#endif
static void set_icache_offset(struct al5_codec_desc *codec)
{
	dma_addr_t dma_handle = codec->icache->dma_handle - MCU_CACHE_OFFSET;
	unsigned long msb = 0;
	u32 icache_offset_lsb;
	u32 icache_offset_msb;

	if (codec->icache->dma_handle < MCU_CACHE_OFFSET)
		msb = 0xFFFFFFFF;

	icache_offset_lsb = (u32)dma_handle;
	icache_offset_msb = (sizeof(dma_handle) == 4) ? msb : dma_handle >> 32;
	al5_writel(icache_offset_lsb, AL5_ICACHE_ADDR_OFFSET_LSB);
	al5_writel(icache_offset_msb, AL5_ICACHE_ADDR_OFFSET_MSB);
}

static void set_dcache_offset(struct al5_codec_desc *codec)
{
	dma_addr_t mem_seg = codec->icache->dma_handle & 0xffffffff00000000;
	dma_addr_t dma_handle = mem_seg - MCU_CACHE_OFFSET;
	unsigned long msb = 0;
	u32 dcache_offset_lsb;
	u32 dcache_offset_msb;

	if (mem_seg < MCU_CACHE_OFFSET)
		msb = 0xFFFFFFFF;

	dcache_offset_lsb = (u32)dma_handle;
	dcache_offset_msb = (sizeof(dma_handle) == 4) ? msb : dma_handle >> 32;

	al5_writel(dcache_offset_msb, AL5_DCACHE_ADDR_OFFSET_MSB);
	al5_writel(dcache_offset_lsb, AL5_DCACHE_ADDR_OFFSET_LSB);
}

static int copy_firmware(struct al5_codec_desc *codec,
			 const struct firmware *fw,
			 const struct firmware *bl_fw)
{
	setup_info("firmware size is %zx", fw->size);
	setup_info("bootloader firmware size is %zx", bl_fw->size);

	if (fw->size > AL5_ICACHE_SIZE) {
		al5_err("firmware is too big");
		return -EINVAL;
	}

	if (bl_fw->size > MCU_SRAM_SIZE) {
		al5_err("bootloader firmware is too big");
		return -EINVAL;
	}

	memcpy(codec->icache->cpu_handle, fw->data, fw->size);
	memcpy_toio_32(codec->regs, bl_fw->data, bl_fw->size);

	return 0;
}

static void stop_mcu(struct al5_codec_desc *codec)
{
	int time = 0;

	al5_writel(0, AL5_MCU_WAKEUP);
	al5_writel(MCU_SLEEP_INSTRUCTION, AL5_MCU_INTERRUPT_HANDLER);
	al5_signal_mcu(codec->users_group.mcu);

	while ((al5_readl(AL5_MCU_STA) & 1) != 1 && time < 100) {
		msleep(20);
		time += 20;
	}
}

static void reset_mcu(struct al5_codec_desc *codec)
{
	/* Mode 1 Reset MCU (sleep) */
	al5_writel(1, AL5_MCU_RESET_MODE);
	al5_writel(0, AL5_MCU_WAKEUP);

	/* Reset MCU */
	al5_writel(1, AL5_MCU_RESET);

}

static void start_mcu(struct al5_codec_desc *codec)
{
	al5_writel(1, AL5_MCU_WAKEUP);
}

static void set_mcu_interrupt_mask(struct al5_codec_desc *codec)
{
	al5_writel(1, AL5_MCU_INTERRUPT_MASK);
}

static int request_all_firmwares(struct al5_codec_desc *codec,
				 const struct firmware **fw,
				 const struct firmware **bl_fw,
				 char *fw_file,
				 char *bl_fw_file)
{
	int err;

	err = request_firmware(fw, fw_file, codec->device);
	if (err) {
		al5_err("firmware file '%s' not found", fw_file);
		goto out_failed;
	}

	err = request_firmware(bl_fw, bl_fw_file, codec->device);
	if (err) {
		al5_err("bootloader firmware file '%s' not found",
			bl_fw_file);
		goto out_failed_firmware;
	}

	return 0;

out_failed_firmware:
	release_firmware(*fw);
out_failed:
	return err;

}

static int setup_and_start_mcu(struct al5_codec_desc *codec,
			       const struct firmware *fw,
			       const struct firmware *bl_fw)
{
	int ret;

	stop_mcu(codec);
	reset_mcu(codec);

	ret = copy_firmware(codec, fw, bl_fw);
	if (ret)
		return ret;

	set_icache_offset(codec);
	set_dcache_offset(codec);
	set_mcu_interrupt_mask(codec);

	start_mcu(codec);

	return 0;
}

static int alloc_mcu_caches(struct al5_codec_desc *codec)
{
	/* alloc the icache and the dcache */
	codec->icache = al5_alloc_dma(codec->device, AL5_ICACHE_SIZE);
	if (!codec->icache)
		return -ENOMEM;

	/* dcache map base addr */

	codec->dcache_base_addr = 0;

	al5_writel(codec->icache->dma_handle >> 32, AXI_ADDR_OFFSET_IP);
	setup_info("icache phy is at %p", (void *)codec->icache->dma_handle);

	return 0;
}

u32 get_l2_size_in_bits(void *);
u32 get_l2_color_bitdepth(void *);
u32 get_num_cores(void *);
u32 get_core_frequency(void *);

static void set_l2_info(struct device *dev, struct mcu_init_msg *init_msg)
{
	void *parent = dev_get_drvdata(dev->parent);

	if (!IS_ERR(parent)) {
		init_msg->l2_size_in_bits = get_l2_size_in_bits(parent);
		init_msg->l2_color_bitdepth = get_l2_color_bitdepth(parent);
		init_msg->num_cores = get_num_cores(parent);
		init_msg->core_frequency = get_core_frequency(parent);
	} else {
		init_msg->l2_size_in_bits = -1;
		init_msg->l2_color_bitdepth = -1;
		init_msg->num_cores = -1;
		init_msg->core_frequency = -1;
	}
}

static int init_mcu(struct al5_codec_desc *codec, struct al5_user *root,
		    bool plugin_enabled)
{
	int err = 0;
	struct mcu_init_msg init_msg;
	struct al5_mail *feedback;

	u32 mcu_memory_pool;
	struct device_node *np = codec->device->of_node;

	err = mutex_lock_killable(&root->locks[AL5_USER_INIT]);
	if (err == -EINTR)
		return err;

	err = al5_queue_pop_timeout(&feedback,
				    &root->queues[AL5_USER_MAIL_INIT]);
	if (err) {
		al5_err("Mcu didn't start (no ping after startup, is mcu enabled ?)");
		goto unlock;
	}
	al5_free_mail(feedback);

	mcu_memory_pool = MCU_SUBALLOCATOR_SIZE;
	of_property_read_u32(np, "al,mcu_ext_mem_size", &mcu_memory_pool);

	codec->suballoc_buf = al5_alloc_dma(codec->device, mcu_memory_pool);
	if (!codec->suballoc_buf) {
		err = -ENOMEM;
		al5_err("Couldn't allocate mcu memory pool");
		goto unlock;
	}

	init_msg.addr = codec->suballoc_buf->dma_handle + MCU_CACHE_OFFSET;
	init_msg.size = codec->suballoc_buf->size;
	set_l2_info(codec->device, &init_msg);
	al5_info("l2 prefetch size:%d (bits), l2 color bitdepth:%d\n",
		 init_msg.l2_size_in_bits, init_msg.l2_color_bitdepth);
	init_msg.plugin_enabled = plugin_enabled;

	err = al5_check_and_send(root, create_init_msg(root->uid, &init_msg));
	if (err) {
		al5_err("Couldn't send initial configuration to mcu");
		goto fail_msg;
	}

	err = al5_queue_pop_timeout(&feedback, &root->queues[AL5_USER_MAIL_INIT]);
	if (err) {
		al5_err("Mcu didn't acknowledge its configuration");
		goto fail_msg;
	}
	al5_free_mail(feedback);

	mutex_unlock(&root->locks[AL5_USER_INIT]);
	return 0;

fail_msg:
	al5_free_dma(codec->device, codec->suballoc_buf);
	codec->suballoc_buf = NULL;
unlock:
	mutex_unlock(&root->locks[AL5_USER_INIT]);
	return err;
}

unsigned int al5_codec_poll(struct file *filp, poll_table *wait)
{
	struct al5_filp_data *private_data = filp->private_data;
	struct al5_user *user = private_data->user;
	unsigned int mask = 0;
	int err;

	struct al5_queue *status = &user->queues[AL5_USER_MAIL_STATUS];
	struct al5_queue *sc = &user->queues[AL5_USER_MAIL_SC];

	poll_wait(filp, &status->queue, wait);
	poll_wait(filp, &sc->queue, wait);

	err = mutex_lock_killable(&user->locks[AL5_USER_CHANNEL]);
	if (err == -EINTR)
		return err;

	if (user->checkpoint == CHECKPOINT_DESTROYED)
		mask = POLLHUP;

	mutex_unlock(&user->locks[AL5_USER_CHANNEL]);

	spin_lock(&status->lock);
	if (!al5_list_empty(status->list))
		mask |= POLLIN;
	spin_unlock(&status->lock);

	spin_lock(&sc->lock);
	if (!al5_list_empty(sc->list))
		mask |= POLLIN;
	spin_unlock(&sc->lock);

	return mask;
}
EXPORT_SYMBOL_GPL(al5_codec_poll);

int al5_codec_open(struct inode *inode, struct file *filp)
{
	struct al5_filp_data *private_data = kzalloc(sizeof(*private_data),
						     GFP_KERNEL);
	struct al5_user *user;
	struct al5_codec_desc *codec;
	int err;

	user = kzalloc(sizeof(*user), GFP_KERNEL);
	if (!user)
		return -ENOMEM;
	codec = container_of(inode->i_cdev, struct al5_codec_desc, cdev);
	user->non_block = filp->f_flags & O_NONBLOCK;

	err = al5_group_bind_user(&codec->users_group, user);
	if (err) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
		kfree_sensitive(user);
#else
		kzfree(user);
#endif
		return err;
	}

	private_data->codec = codec;
	private_data->user = user;
	filp->private_data = private_data;

	return 0;
}
EXPORT_SYMBOL_GPL(al5_codec_open);

void al5_user_destroy_channel_resources(struct al5_user *user);
int al5_codec_release(struct inode *inode, struct file *filp)
{
	struct al5_filp_data *private_data = filp->private_data;
	struct al5_user *user = private_data->user;
	struct al5_codec_desc *codec = private_data->codec;

	if (al5_chan_is_created(user)) {
		const int quiet = 1;

		/* best effort to destroy the channel if it wasn't already */
		int ret = -1;
		int tries = 0;
		while (ret != 0 && tries < 10000) {
			ret = al5_user_destroy_channel(user, quiet);
			++tries;
		}
		if (ret != 0)
			dev_err(codec->device,
				"Failed to destroy channel on mcu. Something went wrong (%d)", ret);

		/* best effort. If everything went wrong, still free the channel resources
		 * to avoid leaks */
		al5_user_destroy_channel_resources(user);
	}
	al5_user_remove_residual_messages(user);
	al5_group_unbind_user(&codec->users_group, user);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
	kfree_sensitive(user);
	kfree_sensitive(filp->private_data);
#else
	kzfree(user);
	kzfree(filp->private_data);
#endif

	return 0;
}
EXPORT_SYMBOL_GPL(al5_codec_release);

int al5_codec_set_firmware(struct al5_codec_desc *codec, char *fw_file,
			   char *bl_fw_file)
{
	const struct firmware *fw = NULL;
	const struct firmware *bl_fw = NULL;
	struct al5_user root;
	int err;
	bool plugin_enabled;

	err = request_all_firmwares(codec, &fw, &bl_fw, fw_file, bl_fw_file);
	if (err)
		return err;

	/* We need to bind root before starting the mcu because we are waiting
	 * for a sync msg
	 */
	err = al5_group_bind_user(&codec->users_group, &root);
	if (err)
		goto release_firmware;

	err = setup_and_start_mcu(codec, fw, bl_fw);
	if (err)
		goto release_firmware;

	/* after this, the mcu is set to send us an interrupt, we can't fail before
	 * ack'ing it */
	plugin_enabled = fw->size > AL5_RC_PLUGIN_CODE_START;

	err = init_mcu(codec, &root, plugin_enabled);
	if (err) {
		stop_mcu(codec);
		goto release_firmware;
	}
	al5_group_unbind_user(&codec->users_group, &root);

release_firmware:
	release_firmware(fw);
	release_firmware(bl_fw);
	return err;
}
EXPORT_SYMBOL_GPL(al5_codec_set_firmware);

int al5_codec_set_up(struct al5_codec_desc *codec, struct platform_device *pdev,
		     size_t max_users_nb)
{
	int err, irq;
	struct device_node *mem_node;
	struct resource mem_res;
	struct resource *res;
	const char *device_name = dev_name(&pdev->dev);
	struct mcu_mailbox_config config;
	struct mcu_mailbox_interface *mcu;
	unsigned long pgtable_padding;

	codec->device = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Can't get resource");
		err = -ENODEV;
		goto fail;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "Failed to get IRQ");
		err = irq;
		goto fail;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
	codec->regs = devm_ioremap(&pdev->dev, res->start, resource_size(res));
#else
	codec->regs = devm_ioremap_nocache(&pdev->dev,
					   res->start, resource_size(res));
#endif
	codec->regs_size = res->end - res->start;

	if (IS_ERR(codec->regs)) {
		dev_err(&pdev->dev, "Can't map registers");
		err = PTR_ERR(codec->regs);
		goto fail;
	}

	mem_node = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
	if (mem_node) {
		err = of_address_to_resource(mem_node, 0, &mem_res);

		if (!err) {
			err = of_reserved_mem_device_init(&pdev->dev);
			if (err) {
				dev_err(&pdev->dev, "Failed to get shared dma pool with error : %d\n", err);
			} else {
				dev_dbg(&pdev->dev, "Using shared dma pool for allocation\n");
				err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
				if (err) {
					dev_err(&pdev->dev, "dma_set_coherent_mask: %d\n", err);
					goto fail_mem;
				}

#ifdef CONFIG_MEMORY_HOTPLUG
			/* Hotplug requires 0x40000000 alignment so round to nearest multiple */
			if (resource_size(&mem_res) % HOTPLUG_ALIGN)
				pgtable_padding = HOTPLUG_ALIGN -
					(resource_size(&mem_res) % HOTPLUG_ALIGN);
			else
				pgtable_padding = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
			add_memory(0, mem_res.start, resource_size(&mem_res) +
					   pgtable_padding, MHP_NONE);
#else
			add_memory(0, mem_res.start, resource_size(&mem_res) +
					   pgtable_padding);
#endif
#endif
			}
		}
	}
	of_node_put(mem_node);

	config.cmd_base = (unsigned long)codec->regs + MAILBOX_CMD;
	config.cmd_size = MAILBOX_SIZE;
	config.status_base = (unsigned long)codec->regs + MAILBOX_STATUS;
	config.status_size = MAILBOX_SIZE;

	err = al5_mcu_interface_create(&mcu, codec->device, &config,
				       codec->regs + AL5_MCU_INTERRUPT);
	if (err) {
		dev_err(&pdev->dev, "Can't create interface with mcu");
		goto fail_mem;
	}

	al5_group_init(&codec->users_group, mcu, max_users_nb, codec->device);

	err = alloc_mcu_caches(codec);
	if (err) {
		dev_err(&pdev->dev, "icache failed to be allocated");
		goto fail_mem;
	}

	err = devm_request_threaded_irq(codec->device,
					irq,
					al5_hardirq_handler,
					al5_irq_handler,
					IRQF_SHARED, device_name, codec);
	if (err) {
		dev_err(&pdev->dev, "Failed to request IRQ #%d -> :%d",
			irq, err);
		goto free_mcu_caches;
	}

	platform_set_drvdata(pdev, codec);

	return 0;

free_mcu_caches:
	al5_free_dma(codec->device, codec->icache);
	codec->icache = NULL;
fail_mem:
	of_reserved_mem_device_release(codec->device);
fail:
	return err;

}
EXPORT_SYMBOL_GPL(al5_codec_set_up);

void al5_codec_tear_down(struct al5_codec_desc *codec)
{
	struct al5_group *group = &codec->users_group;

	al5_mcu_interface_destroy(group->mcu, codec->device);
	al5_group_deinit(group);
	al5_free_dma(codec->device, codec->suballoc_buf);
	al5_free_dma(codec->device, codec->icache);
	of_reserved_mem_device_release(codec->device);
}
EXPORT_SYMBOL_GPL(al5_codec_tear_down);

long al5_codec_compat_ioctl(struct file *file, unsigned int cmd,
			    unsigned long arg)
{
	long ret = -ENOIOCTLCMD;

	if (file->f_op->unlocked_ioctl)
		ret = file->f_op->unlocked_ioctl(file, cmd, arg);

	return ret;
}
EXPORT_SYMBOL_GPL(al5_codec_compat_ioctl);
