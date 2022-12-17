/*
 * al_queue.h
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

#ifndef __AL_QUEUE__
#define __AL_QUEUE__

#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>

#include "al_mailbox.h"
#include "al_list.h"

#define WAIT_TIMEOUT_DURATION (HZ * 5)

struct al5_queue {
	wait_queue_head_t queue;
	struct al5_list *list;
	spinlock_t lock;
	int locked;
};

void al5_queue_init(struct al5_queue *q);
struct al5_mail *al5_queue_pop_no_wait(struct al5_queue *q);
struct al5_mail *al5_queue_pop(struct al5_queue *q);
int al5_queue_pop_timeout(struct al5_mail **mail, struct al5_queue *q);
int al5_queue_push(struct al5_queue *q, struct al5_mail *mail);
void al5_queue_unlock(struct al5_queue *q);
void al5_queue_lock(struct al5_queue *q);

#endif
