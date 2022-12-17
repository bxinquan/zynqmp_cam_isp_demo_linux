// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx zynqmp-dma client driver
 *
 * Copyright (C) 2019 Xilinx, Inc.
 *
 * Author: Jeegar Patel <jeegar.patel@xilinx.com>
 *
 * This driver uses the dma engine subsystem's API to perform
 * memory to memory copy operation using zynqmp-dma and share
 * IOCTL so userspace application use them
 */

#include <linux/cdev.h>
#include <linux/dma-buf.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_dma.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include "dmaproxy.h"

static dev_t dmaproxy_dev;
static struct cdev dmaproxy_cdev;
static struct class *dmaproxy_cl;
#define MINOR_CNT 8

struct dmacopy_done {
	bool		done;
	wait_queue_head_t       wait;
};

struct dmaproxy_data {
	struct dma_chan  *chan;
	struct dma_buf	*src_dbuf;
	struct dma_buf_attachment  *src_attach;
	struct sg_table  *src_sgt;
	struct dma_buf  *dst_dbuf;
	struct dma_buf_attachment  *dst_attach;
	struct sg_table  *dst_sgt;
	dma_addr_t  src_buf;
	dma_addr_t  dst_buf;
	struct dmacopy_done	done;
};


static void dmacopy_callback(void *arg)
{
	struct dmacopy_done *done = arg;

	done->done = true;
	wake_up(&done->wait);
}

static int dmaproxy_open(struct inode *i, struct file *f)
{
	struct dmaproxy_data *dmaproxy_data;
	dma_cap_mask_t mask;
	struct dma_chan *chan;

	dmaproxy_data = kmalloc(sizeof(*dmaproxy_data), GFP_KERNEL);
	if (!dmaproxy_data)
		return -ENOMEM;

	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);

	chan = dma_request_channel(mask, NULL, NULL);
	if (!chan) {
		pr_err("No more DMA channel available\n");
		return -EBUSY;
	}

	dmaproxy_data->chan = chan;

	f->private_data = dmaproxy_data;

	return 0;
}

static int dmaproxy_close(struct inode *i, struct file *f)
{
	struct dmaproxy_data *dmaproxy_data = f->private_data;

	dmaengine_terminate_all(dmaproxy_data->chan);
	dma_release_channel(dmaproxy_data->chan);
	dmaproxy_data->chan = NULL;

	kfree(dmaproxy_data);
	return 0;
}

static int dmabuf_get_address(struct dmaproxy_data *dmaproxy_data, dmaproxy_arg_t dmaproxy)
{
	struct dma_device *dma_dev = dmaproxy_data->chan->device;
	u8  align = 0;

	dmaproxy_data->src_dbuf = dma_buf_get(dmaproxy.src_fd);
	if (IS_ERR(dmaproxy_data->src_dbuf)) {
		pr_err("dma_buf_get failed for src\n");
		goto fail_src_dbuf;
	}

	dmaproxy_data->src_attach = dma_buf_attach(dmaproxy_data->src_dbuf, dma_dev->dev);
	if (IS_ERR(dmaproxy_data->src_attach)) {
		pr_err("dma_buf_attach get failed for src\n");
		goto fail_src_attach;
	}

	dmaproxy_data->src_sgt = dma_buf_map_attachment(dmaproxy_data->src_attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(dmaproxy_data->src_sgt)) {
		pr_err("dma_buf_map get failed for src\n");
		goto fail_src_map;
	}

	dmaproxy_data->src_buf = sg_dma_address(dmaproxy_data->src_sgt->sgl);

	if (dmaproxy.dst_fd != dmaproxy.src_fd) {
		dmaproxy_data->dst_dbuf = dma_buf_get(dmaproxy.dst_fd);
		if (IS_ERR(dmaproxy_data->dst_dbuf)) {
			pr_err("dma_buf_get failed for dst\n");
			goto fail_dst_dbuf;
		}

		dmaproxy_data->dst_attach = dma_buf_attach(dmaproxy_data->dst_dbuf, dma_dev->dev);
		if (IS_ERR(dmaproxy_data->dst_attach)) {
			pr_err("dma_buf_attach get failed for dst\n");
			goto fail_dst_attach;
		}

		dmaproxy_data->dst_sgt = dma_buf_map_attachment(dmaproxy_data->dst_attach, DMA_BIDIRECTIONAL);
		if (IS_ERR(dmaproxy_data->dst_sgt)) {
			pr_err("dma_buf_map get failed for dst\n");
			goto fail_dst_map;
		}

		dmaproxy_data->dst_buf = sg_dma_address(dmaproxy_data->dst_sgt->sgl);
	} else {
		dmaproxy_data->dst_buf = dmaproxy_data->src_buf;
	}

	align = dma_dev->copy_align;
	if (1 << align > dmaproxy.size) {
		pr_err("%lu-byte buffer too small for %d-byte alignment\n",
		       dmaproxy.size, 1 << align);
		goto fail_align;
	}

	dmaproxy_data->src_buf = dmaproxy_data->src_buf + dmaproxy.src_offset;
	dmaproxy_data->dst_buf = dmaproxy_data->dst_buf + dmaproxy.dst_offset;

	if (align)
		dmaproxy_data->src_buf = PTR_ALIGN(dmaproxy_data->src_buf, align);
	if (align)
		dmaproxy_data->dst_buf = PTR_ALIGN(dmaproxy_data->dst_buf, align);

	return 0;

fail_align:
	if (dmaproxy.dst_fd == dmaproxy.src_fd)
		goto fail_dst_dbuf;
	else
		dma_buf_unmap_attachment(dmaproxy_data->dst_attach, dmaproxy_data->dst_sgt, DMA_BIDIRECTIONAL);
fail_dst_map:
	dma_buf_detach(dmaproxy_data->dst_dbuf, dmaproxy_data->dst_attach);
fail_dst_attach:
	dma_buf_put(dmaproxy_data->dst_dbuf);
fail_dst_dbuf:
	dma_buf_unmap_attachment(dmaproxy_data->src_attach, dmaproxy_data->src_sgt, DMA_BIDIRECTIONAL);
fail_src_map:
	dma_buf_detach(dmaproxy_data->src_dbuf, dmaproxy_data->src_attach);
fail_src_attach:
	dma_buf_put(dmaproxy_data->src_dbuf);
fail_src_dbuf:
	return -EINVAL;
}

static void dmabuf_cleanup(struct dmaproxy_data *dmaproxy_data, dmaproxy_arg_t dmaproxy)
{
	dma_buf_unmap_attachment(dmaproxy_data->src_attach, dmaproxy_data->src_sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(dmaproxy_data->src_dbuf, dmaproxy_data->src_attach);
	dma_buf_put(dmaproxy_data->src_dbuf);
	if (dmaproxy.dst_fd != dmaproxy.src_fd) {
		dma_buf_unmap_attachment(dmaproxy_data->dst_attach, dmaproxy_data->dst_sgt, DMA_BIDIRECTIONAL);
		dma_buf_detach(dmaproxy_data->dst_dbuf, dmaproxy_data->dst_attach);
		dma_buf_put(dmaproxy_data->dst_dbuf);
	}
}

static int perform_dma_copy(struct dmaproxy_data *dmaproxy_data, dmaproxy_arg_t dmaproxy)
{
	struct dma_device	*dma_dev;
	struct dma_chan	*chan;
	struct dma_async_tx_descriptor	*tx = NULL;
	struct dmaengine_unmap_data	*um = NULL;
	enum dma_status   status;
	enum dma_ctrl_flags flags;
	dma_cookie_t	 cookie;
	int ret = 0;

	chan = dmaproxy_data->chan;
	dma_dev = dmaproxy_data->chan->device;

	um = dmaengine_get_unmap_data(dma_dev->dev, 2, GFP_KERNEL);
	if (!um) {
		pr_err("dmaengine get unmap data failed\n");
		return -ENOMEM;
	}
	um->len = dmaproxy.size;

	um->addr[0] = dma_map_page(dma_dev->dev, phys_to_page(dmaproxy_data->src_buf),
				   offset_in_page(dmaproxy_data->src_buf), um->len, DMA_TO_DEVICE);
	ret = dma_mapping_error(dma_dev->dev, um->addr[0]);
	if (ret) {
		pr_err("dma map error for source buffer\n");
		goto exit;
	}
	um->to_cnt++;

	um->addr[1] = dma_map_page(dma_dev->dev, phys_to_page(dmaproxy_data->dst_buf),
				   offset_in_page(dmaproxy_data->dst_buf), um->len, DMA_BIDIRECTIONAL);
	ret = dma_mapping_error(dma_dev->dev, um->addr[1]);
	if (ret) {
		pr_err("dma map error for destination buffer\n");
		goto exit;
	}
	um->bidi_cnt++;

	flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
	tx = dma_dev->device_prep_dma_memcpy(chan, um->addr[1], um->addr[0], um->len, flags);
	if (!tx) {
		pr_err("device prep for dma memcpy get failed\n");
		ret = -ECANCELED;
		goto exit;
	}

	dmaproxy_data->done.done = false;
	init_waitqueue_head(&dmaproxy_data->done.wait);
	tx->callback = dmacopy_callback;
	tx->callback_param = &dmaproxy_data->done;
	cookie = tx->tx_submit(tx);

	if (dma_submit_error(cookie)) {
		pr_err("dma submit error\n");
		ret = -ECANCELED;
		goto exit;
	}
	dma_async_issue_pending(chan);

	/*FIXME: Set some X ms of timeout here */
	wait_event_timeout(dmaproxy_data->done.wait, dmaproxy_data->done.done,
			   msecs_to_jiffies(-1));

	status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);

	if (!dmaproxy_data->done.done) {
		pr_err("timeout error while dma copy\n");
		ret = -ETIME;
		goto exit;
	} else if (status != DMA_COMPLETE) {
		pr_err("dma copy get failed\n");
		ret = -ECANCELED;
		goto exit;
	}

exit:
	dmaengine_unmap_put(um);
	return ret;
}

static long dmaproxy_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	dmaproxy_arg_t	dmaproxy;
	struct dmaproxy_data	*dmaproxy_data = f->private_data;
	int	ret;

	switch (cmd) {
	case DMAPROXY_COPY: {
		if (copy_from_user(&dmaproxy, (dmaproxy_arg_t *)arg, sizeof(dmaproxy_arg_t))) {
			pr_err("copy from user failed for DMAPROXY_COPY\n");
			return -EFAULT;
		}

		if (dmabuf_get_address(dmaproxy_data, dmaproxy))
			return -EINVAL;

		ret = perform_dma_copy(dmaproxy_data, dmaproxy);

		dmabuf_cleanup(dmaproxy_data, dmaproxy);
		break;
	}

	default:
		return -EINVAL;
	}

	return ret;
}

static const struct file_operations dmaproxy_fops = {
	.owner = THIS_MODULE,
	.open = dmaproxy_open,
	.release = dmaproxy_close,
	.unlocked_ioctl = dmaproxy_ioctl
};

static int __init dma_proxy_init(void)
{
	int ret;
	struct device *dmaproxy_device;

	ret = alloc_chrdev_region(&dmaproxy_dev, 0, MINOR_CNT, "dmaproxy");
	if (ret < 0)
		return ret;

	cdev_init(&dmaproxy_cdev, &dmaproxy_fops);
	ret = cdev_add(&dmaproxy_cdev, dmaproxy_dev, MINOR_CNT);
	if (ret < 0) {
		unregister_chrdev_region(dmaproxy_dev, MINOR_CNT);
		return ret;
	}

	dmaproxy_cl = class_create(THIS_MODULE, "char");
	if (IS_ERR(dmaproxy_cl)) {
		cdev_del(&dmaproxy_cdev);
		unregister_chrdev_region(dmaproxy_dev, MINOR_CNT);
		return PTR_ERR(dmaproxy_cl);
	}

	dmaproxy_device = device_create(dmaproxy_cl, NULL, dmaproxy_dev, NULL, "dmaproxy");
	if (IS_ERR(dmaproxy_device)) {
		class_destroy(dmaproxy_cl);
		cdev_del(&dmaproxy_cdev);
		unregister_chrdev_region(dmaproxy_dev, MINOR_CNT);
		return PTR_ERR(dmaproxy_device);
	}

	return 0;
}

static void __exit dma_proxy_exit(void)
{
	device_destroy(dmaproxy_cl, dmaproxy_dev);
	class_destroy(dmaproxy_cl);
	cdev_del(&dmaproxy_cdev);
	unregister_chrdev_region(dmaproxy_dev, MINOR_CNT);
}

module_init(dma_proxy_init)
module_exit(dma_proxy_exit)

MODULE_AUTHOR("Jeegar Patel <jeegar.patel@xilinx.com>");
MODULE_DESCRIPTION("Xilinx's zynqmp-dma Client Driver");
MODULE_LICENSE("GPL v2");
