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


#ifndef __AL_CODEC_MAILS__
#define __AL_CODEC_MAILS__

#include "al_mail.h"

enum al5_mail_uid {
	AL_MCU_MSG_INIT,
	AL_MCU_MSG_DEINIT,
	AL_MCU_MSG_IP_INT,
	AL_MCU_MSG_CPU_INT,
	AL_MCU_MSG_CPU_RSP,
	AL_MCU_MSG_CREATE_CHANNEL,
	AL_MCU_MSG_DESTROY_CHANNEL,
	AL_MCU_MSG_ENCODE_ONE_FRM,
	AL_MCU_MSG_DECODE_ONE_FRM,
	AL_MCU_MSG_SEARCH_START_CODE,
	AL_MCU_MSG_QUIET_DESTROY_CHANNEL,
	AL_MCU_MSG_GET_RESOURCE,
	AL_MCU_MSG_SET_BUFFER,
	AL_MCU_MSG_SHUTDOWN,
	AL_MCU_MSG_PUSH_BUFFER_INTERMEDIATE,
	AL_MCU_MSG_PUSH_BUFFER_REFERENCE,

	AL_MCU_MSG_GET_RECONSTRUCTED_PICTURE,
	AL_MCU_MSG_RELEASE_RECONSTRUCTED_PICTURE,
	AL_MCU_MSG_PUT_STREAM_BUFFER,

	/* NSFProd */
	AL_MCU_MSG_TRACE,
	AL_MCU_MSG_RANDOM_TEST,
	AL_MCU_MSG_SET_TIMER_BUFFER,
	AL_MCU_MSG_SET_IRQ_TIMER_BUFFER,

	AL_MCU_MSG_DECODE_ONE_SLICE,

	/* sentinel */
	AL_MCU_MSG_MAX,
};

struct mcu_init_msg {
	u32 addr;
	u32 size;
	u32 l2_size_in_bits;
	u32 l2_color_bitdepth;
	u32 num_cores;
	u32 core_frequency;
	u32 plugin_enabled;
};

struct msg_info {
	u32 chan_uid;
	void *priv;
};

struct al5_mail *create_init_msg(u32 chan_uid, struct mcu_init_msg *msg);
struct al5_mail *create_destroy_channel_msg(u32 chan_uid);
struct al5_mail *create_quiet_destroy_channel_msg(u32 chan_uid);

struct al5_mail *al5_create_classic_mail(u32 sender_uid, u32 msg_uid, void *msg,
					 u32 msg_size);
struct al5_mail *al5_create_empty_mail(u32 sender_uid, u32 msg_uid);

u32 al5_mail_get_chan_uid(struct al5_mail *mail);

void al5_print_mcu_trace(struct al5_mail *mail);

#endif
