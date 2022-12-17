ccflags-y += -I$(src)/../include

obj-$(CONFIG_ALLEGRO) += allegro.o

allegro-objs := \
	al_alloc.o \
	al_alloc_ioctl.o \
	al_buffers_pool.o \
	al_char.o \
	al_codec.o \
	al_dmabuf.o \
	al_user.o \
	al_vcu.o \
	al_mail.o \
	al_codec_mails.o \
	al_mailbox.o \
	al_list.o \
	al_queue.o \
	mcu_interface.o \
	mcu_utils.o \
	al_group.o \

ifeq ($(CONFIG_ALLEGRO_ONLY),1)
  allegro-objs += al_l2_prefetch.o
else
  allegro-objs += xil_l2_prefetch.o
  allegro-objs += xil_clk.o
endif
