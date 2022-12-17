/*
 * al_queue.c
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

#include "al_queue.h"

void al5_queue_init(struct al5_queue *q)
{
	init_waitqueue_head(&q->queue);
	spin_lock_init(&q->lock);
	al5_list_init(&q->list);
	q->locked = true;
}
EXPORT_SYMBOL_GPL(al5_queue_init);

static bool mail_is_available(struct al5_queue *q)
{
	return !al5_list_empty(q->list) || !q->locked;
}

int al5_queue_pop_timeout(struct al5_mail **mail, struct al5_queue *q)
{
	unsigned long flags = 0;

	int err = wait_event_interruptible_timeout(q->queue,
						   mail_is_available(q),
						   WAIT_TIMEOUT_DURATION);

	/*
	 * queue has a strict timeout.
	 * if we get a timeout, it means
	 * the mcu didn't answer and is in
	 * a bad state. we do not want to try again
	 */
	if (err == 0)
		err = -EINVAL;
	else if (err > 0)
		err = 0;
	/* errors of queue are transmitted to the userspace so use EINTR */
	else if (err == -ERESTARTSYS)
		err = -EINTR;

	spin_lock_irqsave(&q->lock, flags);
	*mail = al5_list_pop(&q->list);
	spin_unlock_irqrestore(&q->lock, flags);

	if (*mail)
		err = 0;
	else
		err = (err != 0) ? err : -EINVAL;

	return err;
}
EXPORT_SYMBOL_GPL(al5_queue_pop_timeout);

struct al5_mail *al5_queue_pop(struct al5_queue *q)
{
	struct al5_mail *mail;
	unsigned long flags = 0;

	wait_event_interruptible(q->queue, mail_is_available(q));
	spin_lock_irqsave(&q->lock, flags);
	mail = al5_list_pop(&q->list);
	spin_unlock_irqrestore(&q->lock, flags);

	return mail;
}
EXPORT_SYMBOL_GPL(al5_queue_pop);

struct al5_mail *al5_queue_pop_no_wait(struct al5_queue *q)
{
	struct al5_mail *mail = NULL;
	unsigned long flags = 0;

	if (mail_is_available(q)) {
		spin_lock_irqsave(&q->lock, flags);
		mail = al5_list_pop(&q->list);
		spin_unlock_irqrestore(&q->lock, flags);
	}

	return mail;
}
EXPORT_SYMBOL_GPL(al5_queue_pop_no_wait);

/* return -ENOME if we failed to push, 0 otherwhise */
int al5_queue_push(struct al5_queue *q, struct al5_mail *mail)
{
	unsigned long flags = 0;
	int err;

	spin_lock_irqsave(&q->lock, flags);
	err = al5_list_push(&q->list, mail);
	spin_unlock_irqrestore(&q->lock, flags);

	wake_up_interruptible(&q->queue);

	return err;
}
EXPORT_SYMBOL_GPL(al5_queue_push);

void al5_queue_unlock(struct al5_queue *q)
{
	q->locked = false;
	wake_up_interruptible(&q->queue);
}
EXPORT_SYMBOL_GPL(al5_queue_unlock);

void al5_queue_lock(struct al5_queue *q)
{
	q->locked = true;
}
EXPORT_SYMBOL_GPL(al5_queue_lock);

