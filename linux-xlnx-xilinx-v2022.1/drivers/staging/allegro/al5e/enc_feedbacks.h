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


#ifndef __AL_FEEDBACKS__
#define __AL_FEEDBACKS__

#include <linux/types.h>

struct al5_channel_buffers {
	__u32 int_buffers_count;
	__u32 int_buffers_size;
	__u32 rec_buffers_count;
	__u32 rec_buffers_size;
	__u32 reserved;
};

struct al5e_feedback_channel {
	u32 chan_uid;
	u32 user_uid;
	struct al5_channel_buffers buffers_needed;
	u32 error_code;
};

#endif
