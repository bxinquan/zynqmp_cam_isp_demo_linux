/*
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


#include <linux/device.h>
#include "al_alloc.h"

struct al5_buffer_info {
	u32 bus_address;
	u32 size;
};

void *al5_dmabuf_wrap(struct device *dev, size_t size,
		      struct al5_dma_buffer *buffer);
int al5_create_dmabuf_fd(struct device *dev, size_t size,
			 struct al5_dma_buffer *buffer);

int al5_allocate_dmabuf(struct device *dev, size_t size, u32 *fd);
int al5_dmabuf_get_address(struct device *dev, u32 fd, u32 *bus_address);
int al5_get_dmabuf_info(struct device *dev, u32 fd,
			struct al5_buffer_info *info);
