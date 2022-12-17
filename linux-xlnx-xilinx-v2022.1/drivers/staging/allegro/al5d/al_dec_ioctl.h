/*
 * al_dec_ioctl.h structure used to communicate with the userspace
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

#ifndef _AL_DEC_IOCTL_H_
#define _AL_DEC_IOCTL_H_
#include <linux/types.h>
#include "al_ioctl.h"

#define AL_MCU_CONFIG_CHANNEL  _IOWR('q', 2, struct al5_channel_config)
#define AL_MCU_DESTROY_CHANNEL  _IO('q', 4)
#define AL_MCU_DECODE_ONE_FRM _IOWR('q', 5, struct al5_decode_msg)
#define AL_MCU_WAIT_FOR_STATUS _IOWR('q', 6, struct al5_params)
#define AL_MCU_SEARCH_START_CODE _IOWR('q', 8, struct al5_search_sc_msg)
#define AL_MCU_WAIT_FOR_START_CODE _IOWR('q', 9, struct al5_scstatus)
#define AL_MCU_DECODE_ONE_SLICE _IOWR('q', 18, struct al5_decode_msg)

struct al5_channel_status {
	__u32 error_code;
};

struct al5_params {
	__u32 size;
	__u32 opaque[128];
};

struct al5_channel_config {
	struct al5_params param;
	struct al5_channel_status status;
};

struct al5_scstatus {
	__u16 num_sc;
	__u32 num_bytes;
};

struct al5_decode_msg {
	struct al5_params params;
	struct al5_params addresses;
	__u32 slice_param_v;
};

struct al5_search_sc_msg {
	struct al5_params param;
	struct al5_params buffer_addrs;
};

#endif  /* _AL_DEC_IOCTL_H_ */
