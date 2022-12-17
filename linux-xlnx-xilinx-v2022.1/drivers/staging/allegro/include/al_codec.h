/*
 * al_codec.h per device operation. functions to change the state of the vcu
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

#ifndef _AL_CODEC_H_
#define _AL_CODEC_H_

#include <linux/dma-buf.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/version.h>

#include "al_group.h"
#include "mcu_interface.h"
#include "al_alloc.h"
#include "al_user.h"
#include "al_vcu.h"
#include "al_traces.h"
#include "al_constants.h"

#define al5_writel(val, reg) iowrite32(val, codec->regs + reg)
#define al5_readl(reg) ioread32(codec->regs + reg)

#define al5_dbg(format, ...) \
	dev_dbg(codec->device, format, ## __VA_ARGS__)

#define al5_info(format, ...) \
	dev_info(codec->device, format, ## __VA_ARGS__)

#define al5_err(format, ...) \
	dev_err(codec->device, format, ## __VA_ARGS__)

#define al5_warn_ratelimited(format, ...) \
	dev_warn_ratelimited(codec->device, format, ## __VA_ARGS__)

struct al5_codec_desc {
	struct device *device;
	struct cdev cdev;

	/* Base addr for regs */
	void __iomem *regs;
	unsigned long regs_size;

	/* cache */
	struct al5_dma_buffer *icache, *suballoc_buf;
	dma_addr_t dcache_base_addr;

	struct al5_group users_group;
	int minor;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
	struct clk *pll_ref;
	struct clk *core_clk;
	struct clk *mcu_clk;
	struct regmap *logicore_reg_ba;
#endif
};

struct al5_filp_data {
	struct al5_codec_desc *codec;
	struct al5_user *user;
};

int al5_codec_set_up(struct al5_codec_desc *codec,
		     struct platform_device *pdev,
		     size_t max_users_nb);

void al5_codec_tear_down(struct al5_codec_desc *codec);

int al5_codec_set_firmware(struct al5_codec_desc *codec, char *fw_file,
			   char *bl_fw_file);

int al5_codec_open(struct inode *inode, struct file *filp);
int al5_codec_release(struct inode *inode, struct file *filp);

long al5_codec_compat_ioctl(struct file *file, unsigned int cmd,
			    unsigned long arg);

unsigned int al5_codec_poll(struct file *filp, poll_table *wait);

#endif /* _AL_CODEC_H_ */
