/*
 * al_list.h
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

#ifndef __AL_LIST__
#define __AL_LIST__

#include "al_mailbox.h"

struct al5_list {
	struct al5_mail *mail;
	struct al5_list *next;
};

void al5_list_init(struct al5_list **l);
int al5_list_empty(const struct al5_list *l);
int al5_list_push(struct al5_list **l, struct al5_mail *mail);
struct al5_mail *al5_list_pop(struct al5_list **l);
void al5_list_empty_and_destroy(struct al5_list **l);

#endif
