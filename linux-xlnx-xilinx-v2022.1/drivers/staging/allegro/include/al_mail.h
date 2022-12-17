/*
 * al_mail.h create a message to send to the vcu
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


#ifndef _MCU_COMMON_H_
#define _MCU_COMMON_H_

#include <linux/types.h>

struct al5_mail;

struct al5_mail *al5_mail_create(u32 msg_uid, u32 size);
void al5_mail_write(struct al5_mail *mail, void *content, u32 size);
void al5_mail_write_word(struct al5_mail *mail, u32 word);
void al5_free_mail(struct al5_mail *m_data);
u32 al5_mail_get_uid(struct al5_mail *mail);
u32 al5_mail_get_size(struct al5_mail *mail);
void *al5_mail_get_body(struct al5_mail *mail);
u32 al5_mail_get_word(struct al5_mail *mail, u32 word_offset);

struct al5_mail *al5_mail_create_copy(struct al5_mail *mail);

#endif /* _MCU_COMMON_H_ */
