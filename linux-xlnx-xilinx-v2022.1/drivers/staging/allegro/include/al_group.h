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


#ifndef __AL_GROUP__
#define __AL_GROUP__

#include <linux/spinlock.h>

#include "mcu_interface.h"
#include "al_user.h"

struct al5_group {
	struct mcu_mailbox_interface *mcu;
	spinlock_t lock;

	int max_users_nb;
	struct al5_user **users;
	struct device *device;
};

void al5_group_init(struct al5_group *group, struct mcu_mailbox_interface *mcu,
		    int max_users_nb, struct device *device);
void al5_group_deinit(struct al5_group *group);

int al5_group_bind_user(struct al5_group *group, struct al5_user *user);
void al5_group_unbind_user(struct al5_group *group, struct al5_user *user);

struct al5_user *al5_group_user_from_uid(struct al5_group *group, int user_uid);
struct al5_user *al5_group_user_from_chan_uid(struct al5_group *group,
					      int chan_uid);

void al5_group_read_mails(struct al5_group *group);

#endif
