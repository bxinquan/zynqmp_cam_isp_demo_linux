#ifndef DMAPROXY_IOCTL_H
#define DMAPROXY_IOCTL_H
#include <linux/ioctl.h>

typedef struct dmaproxy_arg {
	int src_fd;
	int dst_fd;
	int src_offset;
	int dst_offset;
	size_t size;
} dmaproxy_arg_t;

#define DMAPROXY_IOCTL_MAGIC 0x32
#define DMAPROXY_COPY _IOWR(DMAPROXY_IOCTL_MAGIC, 1, dmaproxy_arg_t *)

#endif
