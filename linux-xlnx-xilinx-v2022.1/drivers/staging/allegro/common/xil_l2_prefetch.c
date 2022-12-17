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


#include <linux/types.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <linux/mfd/syscon/xlnx-vcu.h>
#else
#include <soc/xilinx/xlnx_vcu.h>
#endif

#define MEMORY_WORD_SIZE_IN_BITS 32

u32 get_l2_color_bitdepth(void *parent)
{
	struct xvcu_device *xvcu = (struct xvcu_device *)parent;

	return xvcu_get_color_depth(xvcu) == 0 ? 8 : 10;
}

static u32 get_memory_words_in_bits(struct xvcu_device *xvcu)
{
	return get_l2_color_bitdepth(xvcu) * MEMORY_WORD_SIZE_IN_BITS;
}

u32 get_l2_size_in_bits(void *parent)
{
	struct xvcu_device *xvcu = (struct xvcu_device *)parent;

	return xvcu_get_memory_depth(xvcu) * get_memory_words_in_bits(xvcu);
}

u32 get_num_cores(void *parent)
{
	struct xvcu_device *xvcu = (struct xvcu_device *)parent;

	u32 num_cores = xvcu_get_num_cores(xvcu);

	if (num_cores == 0)
		return -1;
	return num_cores;
}

u32 get_core_frequency(void *parent)
{
	struct xvcu_device *xvcu = (struct xvcu_device *)parent;

	return xvcu_get_clock_frequency(xvcu);
}
