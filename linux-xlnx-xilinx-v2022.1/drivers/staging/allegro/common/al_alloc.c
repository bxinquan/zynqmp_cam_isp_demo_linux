/*
 * al_alloc.c dma buffers allocation and dmabuf support
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

#include <linux/dma-buf.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "al_alloc.h"

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Kevin Grandemange");
MODULE_AUTHOR("Sebastien Alaiwan");
MODULE_AUTHOR("Antoine Gruzelle");
MODULE_DESCRIPTION("Allegro Common");

struct al5_dma_buffer *al5_alloc_dma_(struct device *dev, size_t size)
{
	struct al5_dma_buffer *buf =
		kmalloc(sizeof(struct al5_dma_buffer),
			GFP_KERNEL);

	if (!buf)
		return NULL;

	buf->offset = 0;
	buf->size = size;
	buf->cpu_handle = dma_alloc_coherent(dev, buf->size,
					     &buf->dma_handle,
					     GFP_KERNEL | GFP_DMA | __GFP_NOWARN);

	if (!buf->cpu_handle) {
		kfree(buf);
		return NULL;
	}

	return buf;
}

struct al5_dma_buffer *al5_alloc_dma(struct device *dev, size_t size)
{
	struct al5_dma_buffer *buf = al5_alloc_dma_(dev, size);

	if (!buf)
		return NULL;

	if ((buf->dma_handle & 0xffffffff) != 0)
		return buf;

	/* Deal with special case of a buffer with lsb == 0 (not accepted by the firmware)
	   Retry the allocation with size + PAGE_SIZE and if we get lsb == 0 again, add a one page
	   offset to the start of the buffer */
	dma_free_coherent(dev, buf->size, buf->cpu_handle, buf->dma_handle);
	buf = al5_alloc_dma_(dev, size + PAGE_SIZE);
	if (!buf)
		return NULL;

	if ((buf->dma_handle & 0xffffffff) == 0) {
		buf->offset = PAGE_SIZE;
		buf->size -= PAGE_SIZE;
		buf->cpu_handle += PAGE_SIZE;
		buf->dma_handle += PAGE_SIZE;
	}

	return buf;
}
EXPORT_SYMBOL_GPL(al5_alloc_dma);

void al5_free_dma(struct device *dev, struct al5_dma_buffer *buf)
{
	if (!buf)
		return;

	buf->cpu_handle -= buf->offset;
	buf->dma_handle -= buf->offset;
	buf->size += buf->offset;
	dma_free_coherent(dev, buf->size, buf->cpu_handle,
			  buf->dma_handle);
	kfree(buf);
}
EXPORT_SYMBOL_GPL(al5_free_dma);


