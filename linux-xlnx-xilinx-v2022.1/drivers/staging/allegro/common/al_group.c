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


#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/string.h>

#include "al_group.h"
#include "al_mail.h"
#include "al_codec_mails.h"
#include "al_constants.h"

void al5_group_init(struct al5_group *group, struct mcu_mailbox_interface *mcu,
		    int max_users_nb, struct device *device)
{
	group->users = kcalloc(max_users_nb, sizeof(void *), GFP_KERNEL);
	group->max_users_nb = max_users_nb;
	spin_lock_init(&group->lock);
	group->mcu = mcu;
	group->device = device;
}

void al5_user_destroy_channel_resources(struct al5_user *user);
void al5_group_deinit(struct al5_group *group)
{
	kfree(group->users);
}

int al5_group_bind_user(struct al5_group *group, struct al5_user *user)
{
	unsigned long flags = 0;
	int uid = -1;
	int err = 0;
	int i;

	spin_lock_irqsave(&group->lock, flags);
	for (i = 0; i < group->max_users_nb; ++i) {
		if (group->users[i] == NULL) {
			uid = i;
			break;
		}
	}
	if (uid == -1) {
		dev_err(group->device, "Max user allocation reached\n");
		err = -EAGAIN;
		goto unlock;
	}

	group->users[uid] = user;
	al5_user_init(user, uid, group->mcu, group->device);

unlock:
	spin_unlock_irqrestore(&group->lock, flags);
	return err;

}
EXPORT_SYMBOL_GPL(al5_group_bind_user);

void al5_group_unbind_user(struct al5_group *group, struct al5_user *user)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&group->lock, flags);
	group->users[user->uid] = NULL;
	spin_unlock_irqrestore(&group->lock, flags);
}
EXPORT_SYMBOL_GPL(al5_group_unbind_user);

struct al5_user *al5_group_user_from_uid(struct al5_group *group, int user_uid)
{
	struct al5_user *user;

	if (user_uid >= group->max_users_nb || user_uid < 0) {
		dev_err(group->device,
			"Received user id %d, expected a value in [0..%ld]\n",
			user_uid, (unsigned long)group->max_users_nb);
		user = NULL;
	} else
		user = group->users[user_uid];

	return user;
}
EXPORT_SYMBOL_GPL(al5_group_user_from_uid);

static bool hasChannelId(struct al5_user *user, int channel_id)
{
	return user != NULL && user->chan_uid == channel_id;
}

struct al5_user *al5_group_user_from_chan_uid(struct al5_group *group,
					      int chan_uid)
{
	int i;

	for (i = 0; i < group->max_users_nb; ++i)
		if (hasChannelId(group->users[i], chan_uid))
			return group->users[i];

	dev_err(group->device, "Received chan_uid %d. No corresponding channel",
		chan_uid);

	return NULL;
}
EXPORT_SYMBOL_GPL(al5_group_user_from_chan_uid);

static void destroy_orphan_channel(struct al5_group *group, int chan_uid)
{
	int err;
	struct al5_mail *mail;

	mail = create_quiet_destroy_channel_msg(chan_uid);
	if (!mail)
		goto fail;

	err = al5_mcu_send(group->mcu, mail);
	if (err)
		goto fail_send;

	al5_signal_mcu(group->mcu);
	al5_free_mail(mail);
	return;

fail_send:
	al5_free_mail(mail);
fail:
	dev_err(group->device, "Couldn't destroy orphan mcu channel\n");
}

void print_cannot_retrieve_user(struct device *dev, u32 mail_uid)
{
	dev_err(dev, "Mail of uid %d received related to inexistant user\n",
		mail_uid);
}

bool should_destroy_channel_on_bad_feedback(u32 mail_uid)
{
	switch (mail_uid) {
	case AL_MCU_MSG_INIT:
	case AL_MCU_MSG_SEARCH_START_CODE:
	case AL_MCU_MSG_DESTROY_CHANNEL:
		return false;
	default:
		return true;
	}
}

struct al5_user *retrieve_user(struct al5_group *group, struct al5_mail *mail)
{
	struct al5_user *user;
	int user_uid, chan_uid = BAD_CHAN;

	switch (al5_mail_get_uid(mail)) {

	case AL_MCU_MSG_CREATE_CHANNEL:
		user_uid = al5_mail_get_word(mail, 1);
		user = al5_group_user_from_uid(group, user_uid);
		break;
	case AL_MCU_MSG_INIT:
	case AL_MCU_MSG_SEARCH_START_CODE:
		user_uid = al5_mail_get_word(mail, 0);
		user = al5_group_user_from_uid(group, user_uid);
		break;
	default:
		chan_uid = al5_mail_get_word(mail, 0);
		user = al5_group_user_from_chan_uid(group, chan_uid);
		break;
	}
	return user;
}

/* return -1 if we failed to retrieve the user, -ENOMEM if there was a memory
 * shortage and 0 otherwhise */
#define AL_NO_USER -1
int try_to_deliver_to_user(struct al5_group *group, struct al5_mail *mail)
{
	struct al5_user *user;
	int ret = AL_NO_USER;
	u32 mail_uid = al5_mail_get_uid(mail);

	spin_lock(&group->lock);
	user = retrieve_user(group, mail);

        if (user == NULL) {
		print_cannot_retrieve_user(group->device, al5_mail_get_uid(mail));
		goto unlock;
	}

	if (mail_uid == AL_MCU_MSG_DESTROY_CHANNEL)
		user->chan_uid = BAD_CHAN;

	if (mail_uid == AL_MCU_MSG_CREATE_CHANNEL)
		user->chan_uid = al5_mail_get_chan_uid(mail);

	ret = al5_user_deliver(user, mail);

unlock:
	spin_unlock(&group->lock);
	return ret;
}

void handle_mail(struct al5_group *group, struct al5_mail *mail)
{
	int error;
	u32 mail_uid = al5_mail_get_uid(mail);

	if (mail_uid == AL_MCU_MSG_TRACE) {
		al5_print_mcu_trace(mail);
		al5_free_mail(mail);
		return;
	}

	error = try_to_deliver_to_user(group, mail);
	if (error == AL_NO_USER) {
		if (should_destroy_channel_on_bad_feedback(mail_uid))
			destroy_orphan_channel(group, al5_mail_get_chan_uid(mail));
		al5_free_mail(mail);
	}else if (error == -ENOMEM)
		dev_err(group->device,
			"Failed to deliver mail to the user (memory shortage). Skipping...\n");
}

void read_mail(struct al5_group *group)
{
	struct al5_mail *mail = al5_mcu_recv(group->mcu);

	if (!mail) {
		dev_err(group->device, "Failed to get mcu mail. Skipping...\n");
		return;
	}

	handle_mail(group, mail);
}

/* A message contains at least a header (2 u16).
   We compute the upper bound of the message number
   to check if the mailbox is broken and stop looping if it is */
#define MAX_MAILBOX_MSG (MAILBOX_SIZE / 4)
void al5_group_read_mails(struct al5_group *group)
{
	unsigned nb_msg = 0;

	while (!al5_mcu_is_empty(group->mcu) && nb_msg++ < MAX_MAILBOX_MSG)
		read_mail(group);
}
