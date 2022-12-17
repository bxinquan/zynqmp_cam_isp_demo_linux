/*
 * mcu_utils.c memory copy using only 32 bits aligned access
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
#include "mcu_utils.h"
/* only 32 bits access are available on the apb */
int memcpy_toio_32(void *pdst, const void *psrc, int size)
{
	const __u32 *src = psrc;
	__u32 *dst = pdst;
	int i;

	for (i = 0; i < size / 4; ++i)
		iowrite32(src[i], dst + i);
	return 0;
}

int memcpy_fromio_32(void *pdst, void *psrc, int size)
{
	__u32 *src = psrc;
	__u32 *dst = pdst;
	int i;

	for (i = 0; i < size / 4; ++i)
		dst[i] = ioread32(src + i);
	return 0;
}

