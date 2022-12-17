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


#ifndef __AL_MAIL_PRIVATE__
#define __AL_MAIL_PRIVATE__

struct al5_mail {
	u32 body_offset;
	u16 msg_uid;
	u16 body_size;
	u8 *body;
};

size_t al5_get_mail_alloc_size(u32 content_size);
void al5_mail_init(struct al5_mail *mail, u32 msg_uid, u32 content_size);

#endif
