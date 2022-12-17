/*
 * al_ioctl.h common ioctl. allocation ioctl
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

#ifndef _AL_IOCTL_H_
#define _AL_IOCTL_H_

#define GET_DMA_FD        _IOWR('q', 13, struct al5_dma_info)
#define GET_DMA_PHY       _IOWR('q', 18, struct al5_dma_info)

#include <linux/types.h>

struct al5_dma_info {
	__u32 fd;
	__u32 size;
	__u32 phy_addr;
};

#endif /* _AL_IOCTL_H_ */
