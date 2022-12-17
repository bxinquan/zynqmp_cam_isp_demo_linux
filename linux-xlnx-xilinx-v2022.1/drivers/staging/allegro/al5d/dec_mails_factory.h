/*
 * dec_mails_factory.h high level message creation for the mcu
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

#ifndef _MCU_DEC_H_
#define _MCU_DEC_H_

#include "mcu_interface.h"
#include "al_mail.h"
#include "al_dec_ioctl.h"

void al5d_mail_get_status(struct al5_params *status, struct al5_mail *mail);
void al5d_mail_get_sc_status(struct al5_scstatus *scstatus,
			     struct al5_mail *mail);

struct al5_mail *al5d_create_decode_one_frame_msg(u32 chan_uid,
						  struct al5_decode_msg *msg);
struct al5_mail *al5d_create_channel_param_msg(u32 user_uid,
					       struct al5_params *msg);
struct al5_mail *al5d_create_search_sc_mail(u32 user_uid,
					    struct al5_search_sc_msg *msg);
struct al5_mail *al5d_create_decode_one_slice_msg(u32 chan_uid,
						  struct al5_decode_msg *msg);

#endif /* _MCU_DEC_H_ */
