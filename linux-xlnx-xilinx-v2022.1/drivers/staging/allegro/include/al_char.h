/*
 * al_char.h char device setup helpers
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
#ifndef _AL_CHAR_H_
#define _AL_CHAR_H_

int al5_setup_chrdev_region(int *major, int minor, int nb_devs, char *desc);
int al5_setup_codec_cdev(struct al5_codec_desc *codec,
			 const struct file_operations *fops,
			 struct module *owner, int major, int minor);
void al5_clean_up_codec_cdev(struct al5_codec_desc *codec);

#endif /* _AL_CHAR_H_ */
