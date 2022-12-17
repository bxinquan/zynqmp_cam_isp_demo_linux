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


#include <linux/printk.h>

#include "al_codec_mails.h"
#include "al_ioctl.h"
#include "al_traces.h"

const u32 uid_size = 4;
struct al5_mail *create_init_msg(u32 chan_uid, struct mcu_init_msg *msg)
{
	struct al5_mail *mail;

	mail = al5_mail_create(AL_MCU_MSG_INIT, uid_size + sizeof(*msg));
	if (!mail)
		return NULL;
	al5_mail_write_word(mail, chan_uid);
	al5_mail_write(mail, msg, sizeof(*msg));

	return mail;
}

struct al5_mail *create_destroy_channel_msg(u32 chan_uid)
{
	return al5_create_empty_mail(chan_uid, AL_MCU_MSG_DESTROY_CHANNEL);
}

struct al5_mail *create_quiet_destroy_channel_msg(u32 chan_uid)
{
	return al5_create_empty_mail(chan_uid,
				     AL_MCU_MSG_QUIET_DESTROY_CHANNEL);
}

struct al5_mail *al5_create_empty_mail(u32 sender_uid, u32 msg_uid)
{
	struct al5_mail *mail = al5_mail_create(msg_uid, uid_size);

	al5_mail_write_word(mail, sender_uid);
	return mail;
}
EXPORT_SYMBOL_GPL(al5_create_empty_mail);
struct al5_mail *al5_create_classic_mail(u32 sender_uid, u32 msg_uid, void *msg,
					 u32 msg_size)
{
	struct al5_mail *mail;

	mail = al5_mail_create(msg_uid, uid_size + msg_size);
	if (!mail)
		return NULL;
	al5_mail_write_word(mail, sender_uid);
	al5_mail_write(mail, msg, msg_size);

	return mail;
}
EXPORT_SYMBOL_GPL(al5_create_classic_mail);


u32 al5_mail_get_chan_uid(struct al5_mail *mail)
{
	u32 mail_uid = al5_mail_get_uid(mail);

	switch (mail_uid) {
	case AL_MCU_MSG_INIT:
	case AL_MCU_MSG_SEARCH_START_CODE:
	case AL_MCU_MSG_TRACE:
		pr_err("assert: mail of uid %d doesn't have a chan_uid\n",
		       mail_uid);
		return -1;
	default:
		return al5_mail_get_word(mail, 0);
	}
}
EXPORT_SYMBOL_GPL(al5_mail_get_chan_uid);

void al5_print_mcu_trace(struct al5_mail *mail)
{
	mcu_info("Mcu trace: %s", (char *)al5_mail_get_body(mail));
}
EXPORT_SYMBOL_GPL(al5_print_mcu_trace);
