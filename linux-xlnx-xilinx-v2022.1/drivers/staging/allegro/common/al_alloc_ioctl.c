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


#include "al_alloc_ioctl.h"
#include "al_dmabuf.h"

#include <linux/uaccess.h>

int al5_ioctl_get_dma_fd(struct device *dev, unsigned long arg)
{
	struct al5_dma_info info;
	int err;

	if (copy_from_user(&info, (struct al5_dma_info *)arg, sizeof(info)))
		return -EFAULT;

	err = al5_allocate_dmabuf(dev, info.size, &info.fd);
	if (err)
		return err;

	err = al5_dmabuf_get_address(dev, info.fd, &info.phy_addr);
	if (err)
		return err;

	if (copy_to_user((void *)arg, &info, sizeof(info)))
		return -EFAULT;

	return 0;
}
EXPORT_SYMBOL_GPL(al5_ioctl_get_dma_fd);

int al5_ioctl_get_dmabuf_dma_addr(struct device *dev, unsigned long arg)
{
	struct al5_dma_info info;
	int err;

	if (copy_from_user(&info, (struct al5_dma_info *)arg, sizeof(info)))
		return -EFAULT;

	err = al5_dmabuf_get_address(dev, info.fd, &info.phy_addr);
	if (err)
		return err;

	if (copy_to_user((void *)arg, &info, sizeof(info)))
		return -EFAULT;

	return 0;
}
EXPORT_SYMBOL_GPL(al5_ioctl_get_dmabuf_dma_addr);
