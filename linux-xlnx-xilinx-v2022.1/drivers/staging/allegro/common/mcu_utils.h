/*
 * mcu_utils.h memory operation using only 32 bits aligned accesses
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

#ifndef _MCU_UTILS_H_
#define _MCU_UTILS_H_

#include <linux/types.h>
#include <linux/io.h>

int memcpy_toio_32(void *pdst, const void *psrc, int size);
int memcpy_fromio_32(void *pdst, void *psrc, int size);

#endif /* _MCU_UTILS_H_ */
