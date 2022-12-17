/*
 * al_traces.h
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

#ifndef _AL_TRACES_H_
#define _AL_TRACES_H_

#include <linux/printk.h>

#define AL5_DEBUG 0
#if AL5_DEBUG
#define mails_info(format, ...) \
	do {    \
		pr_info("\e[0;32m"); \
		pr_cont(format, ## __VA_ARGS__); \
		pr_cont("\e[0m\n"); \
	} while (0)
#define ioctl_info(format, ...) \
	do {    \
		pr_info("\e[0;33m"); \
		pr_cont(format, ## __VA_ARGS__); \
		pr_cont("\e[0m\n"); \
	} while (0)

#define irq_info(format, ...) \
	do { \
		pr_info("\e[0;34m"); \
		pr_cont(format, ## __VA_ARGS__); \
		pr_cont("\e[0m\n"); \
	} while (0)

#define setup_info(format, ...) \
	dev_info(codec->device, format, ## __VA_ARGS__)
#else

#define mails_info(format, ...)
#define ioctl_info(format, ...)
#define irq_info(format, ...)
#define setup_info(format, ...)

#endif

#define mcu_info(format, ...) \
	do {    \
		pr_info("\e[0;31m"); \
		pr_cont(format, ## __VA_ARGS__); \
		pr_cont("\e[0m\n"); \
	} while (0)

#endif
