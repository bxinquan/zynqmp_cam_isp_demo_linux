/*
 * al_vcu.h handle communication with mcu via interrupts
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

#ifndef _AL_VCU_H_
#define _AL_VCU_H_

#include <linux/interrupt.h>

#define AL5_NR_DEVS 4
#define MAX_USERS_NB 256

//extern int max_users_nb;

irqreturn_t al5_irq_handler(int irq, void *data);
irqreturn_t al5_hardirq_handler(int irq, void *data);


#endif /* _AL_VCU_H_ */
