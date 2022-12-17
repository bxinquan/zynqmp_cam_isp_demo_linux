/*
 * mcu_interface.h mcu interface of the vcu. implements access control to the
 * mailbox and the protocol to signal a message was sent to the mcu.
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

#ifndef _MCU_INTERFACE_H_
#define _MCU_INTERFACE_H_

#include <linux/slab.h>
#include <linux/device.h>

#include "al_mailbox.h"
#include "al_mail.h"

struct mcu_mailbox_interface;

struct mcu_mailbox_config {
	unsigned long cmd_base;
	unsigned long status_base;
	unsigned long cmd_size;
	unsigned long status_size;
};

int al5_mcu_interface_create(struct mcu_mailbox_interface **mcu,
			     struct device *device,
			     struct mcu_mailbox_config *config,
			     void *mcu_interrupt_register);

void al5_mcu_interface_destroy(struct mcu_mailbox_interface *mcu,
			       struct device *device);

int al5_mcu_is_empty(struct mcu_mailbox_interface *mcu);

int al5_mcu_send(struct mcu_mailbox_interface *mcu, struct al5_mail *data);
struct al5_mail *al5_mcu_recv(struct mcu_mailbox_interface *mcu);

void al5_signal_mcu(struct mcu_mailbox_interface *mcu);

u32 al5_mcu_get_virtual_address(u32 physicalAddress);

#endif /* _MCU_INTERFACE_H_ */
