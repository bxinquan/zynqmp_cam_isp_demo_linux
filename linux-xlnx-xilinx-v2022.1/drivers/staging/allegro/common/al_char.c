/*
 * al_char.c char device setup helpers
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

#include "al_codec.h"

int al5_setup_chrdev_region(int *major, int base_minor, int nb_devs, char *desc)
{
	dev_t dev = 0;
	int err;

	if (*major == 0) {
		err = alloc_chrdev_region(&dev, base_minor, nb_devs, desc);
		*major = MAJOR(dev);

		if (err) {
			pr_alert("Allegro codec: can't get major %d\n",
				 *major);
			return err;
		}
	}
	return 0;
}
EXPORT_SYMBOL_GPL(al5_setup_chrdev_region);

int al5_setup_codec_cdev(struct al5_codec_desc *codec,
			 const struct file_operations *fops,
			 struct module *owner, int major, int minor)
{
	int err, devno = MKDEV(major, minor);

	cdev_init(&codec->cdev, fops);
	codec->cdev.owner = owner;
	err = cdev_add(&codec->cdev, devno, 1);
	if (err) {
		al5_err("Error %d adding allegro device number %d", err, minor);
		return err;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(al5_setup_codec_cdev);

void al5_clean_up_codec_cdev(struct al5_codec_desc *codec)
{
	cdev_del(&codec->cdev);
}
EXPORT_SYMBOL_GPL(al5_clean_up_codec_cdev);


