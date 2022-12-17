/*
 * enc_user.h high level communication with the vcu and the firmware
 * channel creation, frame encoding
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
#include "al_user.h"
#include "al_enc_ioctl.h"

int al5e_user_create_channel(struct al5_user *user,
			     struct al5_config_channel *msg);
int al5e_user_encode_one_frame(struct al5_user *user,
			       struct al5_encode_msg *msg);
int al5e_user_wait_for_status(struct al5_user *user,
			      struct al5_params *msg);
int al5e_user_put_stream_buffer(struct al5_user *user,
				struct al5_buffer *buffer);

int al5e_user_release_rec(struct al5_user *user, u32 fd);
int al5e_user_get_rec(struct al5_user *user,
		      struct al5_reconstructed_info *msg);
