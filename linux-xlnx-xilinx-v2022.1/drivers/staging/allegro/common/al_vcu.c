/*
 * al_vcu.c handle communication with mcu via interrupts
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

#include "al_vcu.h"
#include "al_codec.h"
#include "al_group.h"

#define AL5_MCU_IRQ_STA 0x910C

irqreturn_t al5_hardirq_handler(int irq, void *data)
{
	struct al5_codec_desc *codec = (struct al5_codec_desc *)data;
	u32 irq_status = al5_readl(AL5_MCU_IRQ_STA);

	if (irq_status == 0)
		return IRQ_NONE;

	al5_writel(irq_status, AL5_MCU_INTERRUPT_CLR);

	/* wait for the interrupt acknowledgment to propagate in the hw */
	al5_readl(AL5_MCU_IRQ_STA);

	return IRQ_WAKE_THREAD;
}
EXPORT_SYMBOL_GPL(al5_hardirq_handler);

irqreturn_t al5_irq_handler(int irq, void *data)
{
	struct al5_codec_desc *codec = (struct al5_codec_desc *)data;

	irq_info("Got irq from Mcu");
	al5_group_read_mails(&codec->users_group);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL_GPL(al5_irq_handler);

