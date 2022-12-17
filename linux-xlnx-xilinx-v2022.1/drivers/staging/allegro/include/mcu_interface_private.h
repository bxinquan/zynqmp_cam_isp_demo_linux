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


#ifndef __MCU_INTERFACE_PRIVATE__
#define __MCU_INTERFACE_PRIVATE__

#include <linux/spinlock.h>
#include "mcu_interface.h"

struct mcu_mailbox_interface {
	struct mailbox mcu_to_cpu;
	struct mailbox cpu_to_mcu;
	spinlock_t read_lock;
	spinlock_t write_lock;
	void *interrupt_register;
	struct device *dev;
};

#endif
