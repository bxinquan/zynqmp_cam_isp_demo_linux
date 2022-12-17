/*
 * al_buffers_pool.h high level function to write messages in the mailbox
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

#ifndef __AL_BUFFERS_POOL__
#define __AL_BUFFERS_POOL__

struct al5_buffers_pool {
	int count;
	struct al5_dma_buffer **buffers;
	void **handles;
	int *fds;
};


void al5_bufpool_init(struct al5_buffers_pool *bufpool);
int al5_bufpool_allocate(struct al5_buffers_pool *bufpool,
			 struct device *device, int count, int size);
void al5_bufpool_free(struct al5_buffers_pool *bufpool, struct device *device);
int al5_bufpool_get_id(struct al5_buffers_pool *bufpool, int fd);
int al5_bufpool_reserve_fd(struct al5_buffers_pool *bufpool, int id);

#endif
