/*
 * al_enc_ioctl.h structure used to communicate with the userspace
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

#ifndef _AL_ENC_IOCTL_H_
#define _AL_ENC_IOCTL_H_
#include "al_ioctl.h"
#include <linux/types.h>

#define AL_MCU_CONFIG_CHANNEL  _IOWR('q', 2, struct al5_config_channel)
#define AL_MCU_DESTROY_CHANNEL  _IO('q', 4)
#define AL_MCU_ENCODE_ONE_FRM _IOWR('q', 5, struct al5_encode_msg)
#define AL_MCU_WAIT_FOR_STATUS _IOWR('q', 6, struct al5_params)

#define AL_MCU_PUT_STREAM_BUFFER _IOWR('q', 22, struct al5_buffer)

#define AL_MCU_GET_REC_PICTURE _IOWR('q', 23, struct al5_reconstructed_info)
#define AL_MCU_RELEASE_REC_PICTURE _IOWR('q', 24, __u32)


struct al5_reconstructed_info {
	__u32 fd;
	__u32 pic_struct;
	__u32 poc;
	__u32 width;
	__u32 height;
};

struct al5_params {
	__u32 size;
	__u32 opaque_params[128];
};

struct al5_channel_status {
	__u32 error_code;
};

struct al5_config_channel {
	struct al5_params param;
	struct al5_channel_status status;
	__u32 rc_plugin_fd;
};

struct al5_encode_msg {
	struct al5_params params;
	struct al5_params addresses;
};

struct al5_stream_buffer {
	__u64 stream_buffer_ptr;
	__u32 handle;
	__u32 offset;
	__u32 size;
};

struct al5_buffer {
	struct al5_stream_buffer stream_buffer;
	__u32 external_mv_handle;
};

#endif  /* _AL_ENC_IOCTL_H_ */
