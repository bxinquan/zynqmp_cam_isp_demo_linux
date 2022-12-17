/*
 * enc_mails_factory.c high level message creation for the mcu
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
#include <linux/types.h>
#include <linux/string.h>

#include "enc_mails_factory.h"

void al5e_mail_get_status(struct al5_params *status,
			  struct al5_mail *mail)
{
	status->size = al5_mail_get_size(mail) - 4;
	memcpy(status->opaque_params, al5_mail_get_body(mail) + 4,
	       status->size);
}

struct al5_mail *al5e_create_encode_one_frame_msg(u32 chan_uid,
						  struct al5_encode_msg *msg)
{
	int mail_size = 8 + msg->params.size + msg->addresses.size;
	struct al5_mail *mail = al5_mail_create(AL_MCU_MSG_ENCODE_ONE_FRM,
						mail_size);

	const int padding = 0;

	al5_mail_write_word(mail, chan_uid);
	al5_mail_write_word(mail, padding);
	al5_mail_write(mail, msg->params.opaque_params, msg->params.size);
	al5_mail_write(mail, msg->addresses.opaque_params, msg->addresses.size);

	return mail;
}

struct al5_mail *al5e_create_channel_param_msg(u32 user_uid,
					       struct al5_params *msg,
					       u32 mcu_rc_plugin_addr,
					       u32 mcu_rc_plugin_size)
{
	/* One word for the user uid and two words for the rc plugin */
	struct al5_mail *mail = al5_mail_create(AL_MCU_MSG_CREATE_CHANNEL, msg->size + 3 * sizeof(u32));
	if(!mail)
		return NULL;

	al5_mail_write_word(mail, user_uid);
	al5_mail_write(mail, msg->opaque_params, msg->size);
	al5_mail_write_word(mail, mcu_rc_plugin_addr);
	al5_mail_write_word(mail, mcu_rc_plugin_size);

	return mail;
}

