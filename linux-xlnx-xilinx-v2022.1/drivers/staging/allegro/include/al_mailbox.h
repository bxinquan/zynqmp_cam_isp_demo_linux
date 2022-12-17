/*
 * al_mailbox.h mailbox mechanisms. r/w messages to/from the mcu
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

#ifndef _MCU_MAILBOX_H_
#define _MCU_MAILBOX_H_

#include <linux/types.h>

#include "al_mail.h"

struct mailbox {
	size_t size; /* In bytes */
	u8 *data;
	void *tail;
	void *head;
	u16 local_tail;
};

void al5_mailbox_init(struct mailbox *box, void *base, size_t data_size);
int al5_mailbox_write(struct mailbox *box, struct al5_mail *mail);
bool al5_mailbox_read(struct mailbox *box, struct al5_mail *mail, size_t mail_size);

#endif /* _MCU_MAILBOX_H_ */
