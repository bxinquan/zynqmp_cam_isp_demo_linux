/*
 * al_user.c high level function to write messages in the mailbox
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
#include <linux/kernel.h>

#include "al_user.h"
#include "al_codec_mails.h"

static int mail_to_queue(int mail_uid)
{
	switch (mail_uid) {
	case AL_MCU_MSG_INIT:
		return AL5_USER_MAIL_INIT;
	case AL_MCU_MSG_CREATE_CHANNEL:
		return AL5_USER_MAIL_CREATE;
	case AL_MCU_MSG_DESTROY_CHANNEL:
		return AL5_USER_MAIL_DESTROY;
	case AL_MCU_MSG_ENCODE_ONE_FRM:
	case AL_MCU_MSG_DECODE_ONE_FRM:
		return AL5_USER_MAIL_STATUS;
	case AL_MCU_MSG_SEARCH_START_CODE:
		return AL5_USER_MAIL_SC;
	case AL_MCU_MSG_GET_RECONSTRUCTED_PICTURE:
	case AL_MCU_MSG_RELEASE_RECONSTRUCTED_PICTURE:
		return AL5_USER_MAIL_REC;
	default:
		return AL5_USER_MAIL_DEBUG;
	}
}

static int send_msg(struct mcu_mailbox_interface *mcu, struct al5_mail *mail)
{
	int err = al5_mcu_send(mcu, mail);

	if (err)
		return err;

	al5_signal_mcu(mcu);

	return 0;
}

int al5_check_and_send(struct al5_user *user, struct al5_mail *mail)
{
	int err;

	if (!mail)
		return -ENOMEM;
	err = send_msg(user->mcu, mail);
	al5_free_mail(mail);
	if (err)
		return err;

	return 0;
}
EXPORT_SYMBOL_GPL(al5_check_and_send);

/* return -ENOMEM if we failed to deliver, 0 otherwhise */
int al5_user_deliver(struct al5_user *user, struct al5_mail *mail)
{
	struct al5_queue *queue =
		&user->queues[mail_to_queue(al5_mail_get_uid(mail))];

	return al5_queue_push(queue, mail);
}

static void user_queues_unlock(struct al5_user *user)
{
	al5_queue_unlock(&user->queues[AL5_USER_MAIL_STATUS]);
	al5_queue_unlock(&user->queues[AL5_USER_MAIL_SC]);
	al5_queue_unlock(&user->queues[AL5_USER_MAIL_CREATE]);
	al5_queue_unlock(&user->queues[AL5_USER_MAIL_REC]);
}

static void user_queues_lock(struct al5_user *user)
{
	al5_queue_lock(&user->queues[AL5_USER_MAIL_STATUS]);
	al5_queue_lock(&user->queues[AL5_USER_MAIL_SC]);
	al5_queue_lock(&user->queues[AL5_USER_MAIL_CREATE]);
	al5_queue_lock(&user->queues[AL5_USER_MAIL_REC]);
}

void al5_user_remove_residual_messages(struct al5_user *user)
{
	int queue_id;

	for (queue_id = 0; queue_id < AL5_USER_MAIL_NUMBER; ++queue_id) {
		struct al5_queue *q = &user->queues[queue_id];
		struct al5_mail *mail;

		al5_queue_unlock(q);
		mail = al5_queue_pop(q);

		while (mail) {
			al5_free_mail(mail);
			mail = al5_queue_pop(q);
		}
	}
}

void al5_user_init(struct al5_user *user, int uid,
		   struct mcu_mailbox_interface *mcu, struct device *device)
{
	int i;

	memset(user, 0, sizeof(*user));
	for (i = 0; i < AL5_USER_OPS_NUMBER; ++i)
		mutex_init(&user->locks[i]);
	for (i = 0; i < AL5_USER_MAIL_NUMBER; ++i)
		al5_queue_init(&user->queues[i]);
	user->uid = uid;
	user->mcu = mcu;
	user->chan_uid = BAD_CHAN;
	user->checkpoint = NO_CHECKPOINT;
	user->device = device;
	al5_bufpool_init(&user->int_buffers);
	al5_bufpool_init(&user->rec_buffers);
}
EXPORT_SYMBOL_GPL(al5_user_init);

void al5_user_destroy_channel_resources(struct al5_user *user)
{
	al5_bufpool_free(&user->int_buffers, user->device);
	al5_bufpool_free(&user->rec_buffers, user->device);
}

int al5_user_destroy_channel(struct al5_user *user, int quiet)
{
	int err = 0;
	int i, j;
	struct al5_mail *mail;

	// Destroy the channel if it was created
	if (user->checkpoint == NO_CHECKPOINT)
		return -EPERM;

	user_queues_unlock(user);
	for (i = 0; i < AL5_USER_OPS_NUMBER; ++i) {
		err = mutex_lock_killable(&user->locks[i]);
		if (err == -EINTR) {
			for (j = 0; j < i; ++j)
				mutex_unlock(&user->locks[j]);

			goto relock_user_queues;
		}
	}

	if (quiet) {
		err = al5_check_and_send(user,
					 create_quiet_destroy_channel_msg(
						 user->chan_uid));
		if (err)
			goto unlock_mutexes;
	} else {
		err = al5_check_and_send(user, create_destroy_channel_msg(
						 user->chan_uid));
		if (err)
			goto unlock_mutexes;

		mail = al5_queue_pop(&user->queues[AL5_USER_MAIL_DESTROY]);
		if (mail == NULL) {
			err = -EINTR;
			goto unlock_mutexes;
		}

		al5_free_mail(mail);
	}

	user->checkpoint = CHECKPOINT_DESTROYED;
	al5_user_destroy_channel_resources(user);

unlock_mutexes:
	for (i = 0; i < AL5_USER_OPS_NUMBER; ++i)
		mutex_unlock(&user->locks[i]);
relock_user_queues:
	user_queues_lock(user);
	return err;

}
EXPORT_SYMBOL_GPL(al5_user_destroy_channel);

int al5_chan_is_created(struct al5_user *user)
{
	return user->checkpoint == CHECKPOINT_CREATED;
}
EXPORT_SYMBOL_GPL(al5_chan_is_created);

struct al5_mail *al5_user_get_mail(struct al5_user *user, u32 mail_uid)
{
	struct al5_mail *mail;
	u32 queue_uid = mail_to_queue(mail_uid);

	al5_queue_unlock(&user->queues[queue_uid]);
	mail = al5_queue_pop(&user->queues[queue_uid]);
	al5_queue_lock(&user->queues[queue_uid]);

	return mail;
}
