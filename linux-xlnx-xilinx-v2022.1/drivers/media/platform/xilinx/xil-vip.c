// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Xil VIP IP
 *
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/v4l2-subdev.h>
#include <media/media-entity.h>
#include <media/v4l2-common.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-event.h>

/* Register register map */

#define VIP_REG_RESET                      (0 * 4)
#define VIP_REG_WIDTH                      (1 * 4)
#define VIP_REG_HEIGHT                     (2 * 4)
#define VIP_REG_BITS                       (3 * 4)
#define VIP_REG_TOP_EN                     (4 * 4)
#define VIP_REG_HIST_EQU_MIN               (5 * 4)
#define VIP_REG_HIST_EQU_MAX               (6 * 4)
#define VIP_REG_CROP_X                     (7 * 4)
#define VIP_REG_CROP_Y                     (8 * 4)
#define VIP_REG_CROP_W                     (9 * 4)
#define VIP_REG_CROP_H                     (10 * 4)
#define VIP_REG_DSCALE_H                   (11 * 4)
#define VIP_REG_DSCALE_V                   (12 * 4)
#define VIP_REG_OSD_X                      (13 * 4)
#define VIP_REG_OSD_Y                      (14 * 4)
#define VIP_REG_OSD_W                      (15 * 4)
#define VIP_REG_OSD_H                      (16 * 4)
#define VIP_REG_OSD_RGB_FG                 (17 * 4)
#define VIP_REG_OSD_RGB_BG                 (18 * 4)
#define VIP_REG_YUV444TO422_SWITCH_UV      (19 * 4)
#define VIP_REG_INT_STATUS                 (20 * 4)
#define VIP_REG_INT_MASK                   (21 * 4)

#define VIP_REG_OSD_RAM_ADDR               (1*512*4)
#define VIP_REG_OSD_RAM_SIZE               (1*512*4)

#define VIP_REG_TOP_EN_BIT_HIST_EQU_EN     (1<<0)
#define VIP_REG_TOP_EN_BIT_SOBEL_EN        (1<<1)
#define VIP_REG_TOP_EN_BIT_YUV2RGB_EN      (1<<2)
#define VIP_REG_TOP_EN_BIT_CROP_EN         (1<<3)
#define VIP_REG_TOP_EN_BIT_DSCALE_EN       (1<<4)
#define VIP_REG_TOP_EN_BIT_OSD_EN          (1<<5)
#define VIP_REG_TOP_EN_BIT_YUV444TO422_EN  (1<<6)

#define VIP_REG_INT_STATUS_BIT_FRAME_START (1<<0)
#define VIP_REG_INT_STATUS_BIT_FRAME_DONE  (1<<1)

#define VIP_REG_INT_MASK_BIT_FRAME_START   (1<<0)
#define VIP_REG_INT_MASK_BIT_FRAME_DONE    (1<<1)
////////////////////////////////////

#define VIP_PAD_SINK		0
#define VIP_PAD_SOURCE		1
#define VIP_MEDIA_PADS		2


/**
 * struct vip_state - CSI-2 Rx device structure
 * @subdev: The v4l2 subdev structure
 * @format: Active V4L2 formats on each pad
 * @default_format: Default V4L2 format
 * @dev: Platform structure
 * @clks: array of clocks
 * @iomem: Base address of subsystem
 * @lock: mutex for accessing this structure
 * @pads: media pads
 * @streaming: Flag for storing streaming state
 *
 * This structure contains the device driver related parameters
 */
struct vip_state {
	struct v4l2_subdev subdev;
	struct v4l2_mbus_framefmt pad_format[VIP_MEDIA_PADS];
	struct device *dev;
	struct clk_bulk_data *clks;
	void __iomem *iomem;
	u32 bits;
	/* used to protect access to this struct */
	struct mutex lock;
	struct media_pad pads[VIP_MEDIA_PADS];
	bool streaming;
};

static const struct clk_bulk_data vip_clks[] = {
	{ .id = "s00_axi_aclk" },
	{ .id = "pclk" },
};

static inline struct vip_state *
to_vipstate(struct v4l2_subdev *subdev)
{
	return container_of(subdev, struct vip_state, subdev);
}

/*
 * Register related operations
 */
static inline u32 vip_read(struct vip_state *vip, u32 addr)
{
	return ioread32(vip->iomem + addr);
}

static inline void vip_write(struct vip_state *vip, u32 addr,
				   u32 value)
{
	iowrite32(value, vip->iomem + addr);
}

static int vip_log_status(struct v4l2_subdev *sd)
{
	struct vip_state *vip = to_vipstate(sd);
	struct device *dev = vip->dev;
	u32 width, height, bits;

	mutex_lock(&vip->lock);

	width     = vip_read(vip, VIP_REG_WIDTH);
	height    = vip_read(vip, VIP_REG_HEIGHT);
	bits      = vip_read(vip, VIP_REG_BITS);
	dev_info(dev, "VIP %u x %u YUV%u",
			width, height, bits);

	mutex_unlock(&vip->lock);

	return 0;
}

static int vip_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			       struct v4l2_event_subscription *sub)
{
	if (sub->type != V4L2_EVENT_FRAME_SYNC)
		return -EINVAL;

	/* V4L2_EVENT_FRAME_SYNC doesn't require an id, so zero should be set */
	if (sub->id != 0)
		return -EINVAL;

	return v4l2_event_subscribe(fh, sub, 0, NULL);
}

static int vip_start_stream(struct vip_state *vip)
{
	u32 in_width   = vip->pad_format[VIP_PAD_SINK].width;
	u32 in_height  = vip->pad_format[VIP_PAD_SINK].height;
	u32 out_width  = vip->pad_format[VIP_PAD_SOURCE].width;
	u32 out_height = vip->pad_format[VIP_PAD_SOURCE].height;
	u32 out_code   = vip->pad_format[VIP_PAD_SOURCE].code;

	u32 scale_h = in_width / out_width;
	u32 scale_v = in_height / out_height;

	u32 top_en = 0;

	if (out_code == MEDIA_BUS_FMT_RBG888_1X24) {
		top_en |= VIP_REG_TOP_EN_BIT_YUV2RGB_EN;
	} else {
		top_en |= VIP_REG_TOP_EN_BIT_YUV444TO422_EN;
	}
	if (in_width != out_width || in_height != out_height) {
		top_en |= VIP_REG_TOP_EN_BIT_CROP_EN;
	}
	if (scale_h > 1 && scale_v > 1) {
		top_en |= VIP_REG_TOP_EN_BIT_DSCALE_EN;
	}
	vip_write(vip, VIP_REG_TOP_EN, top_en);

	if (top_en & VIP_REG_TOP_EN_BIT_DSCALE_EN) {
		u32 scale_val = scale_h < scale_v ? scale_h : scale_v;
		vip_write(vip, VIP_REG_CROP_X, (in_width-out_width*scale_val)/2);
		vip_write(vip, VIP_REG_CROP_Y, (in_height-out_height*scale_val)/2);
		vip_write(vip, VIP_REG_CROP_W, out_width*scale_val);
		vip_write(vip, VIP_REG_CROP_H, out_height*scale_val);
		vip_write(vip, VIP_REG_DSCALE_H, scale_val-1);
		vip_write(vip, VIP_REG_DSCALE_V, scale_val-1);
	} else {
		vip_write(vip, VIP_REG_CROP_X, (in_width-out_width)/2);
		vip_write(vip, VIP_REG_CROP_Y, (in_height-out_height)/2);
		vip_write(vip, VIP_REG_CROP_W, out_width);
		vip_write(vip, VIP_REG_CROP_H, out_height);
	}

	vip_write(vip, VIP_REG_INT_STATUS, 0);
	vip_write(vip, VIP_REG_INT_MASK, ~(VIP_REG_INT_MASK_BIT_FRAME_START|VIP_REG_INT_MASK_BIT_FRAME_DONE));
	vip_write(vip, VIP_REG_RESET, 0);

	vip->streaming = true;

	return 0;
}

static void vip_stop_stream(struct vip_state *vip)
{
	vip_write(vip, VIP_REG_RESET, 1);
	vip_write(vip, VIP_REG_INT_MASK, ~0U);
	vip_write(vip, VIP_REG_INT_STATUS, 0);

	vip->streaming = false;
}

static int vip_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct vip_state *vip = to_vipstate(sd);
	int ret = 0;

	mutex_lock(&vip->lock);

	if (enable == vip->streaming) {
		goto stream_done;
	}

	if (enable) {
		ret = vip_start_stream(vip);
	} else {
		vip_stop_stream(vip);
	}

stream_done:
	mutex_unlock(&vip->lock);
	return ret;
}

static void vip_queue_event_frame_sync(struct vip_state *vip, u32 frame_seq)
{
	struct v4l2_event event = {
		.type = V4L2_EVENT_FRAME_SYNC,
	};
	event.u.frame_sync.frame_sequence = frame_seq;

	v4l2_event_queue(vip->subdev.devnode, &event);
}

/**
 * vip_irq_handler - Interrupt handler for CSI-2
 * @irq: IRQ number
 * @data: Pointer to device state
 *
 * In the interrupt handler, a list of event counters are updated for
 * corresponding interrupts. This is useful to get status / debug.
 *
 * Return: IRQ_HANDLED after handling interrupts
 */
static irqreturn_t vip_irq_handler(int irq, void *data)
{
	struct vip_state *vip = (struct vip_state *)data;
	//struct device *dev = vip->dev;
	u32 status;

	status = vip_read(vip, VIP_REG_INT_STATUS);
	vip_write(vip, VIP_REG_INT_STATUS, 0);

	if (status & VIP_REG_INT_STATUS_BIT_FRAME_START) {
		//dev_info(dev, "IRQ FRAME_START");
		vip_queue_event_frame_sync(vip, 0);
	}

	if (status & VIP_REG_INT_STATUS_BIT_FRAME_DONE) {
		//XXX
		//dev_info(dev, "IRQ FRAME_DONE");
	}

	return IRQ_HANDLED;
}

/**
 * vip_init_cfg - Initialise the pad format config to default
 * @sd: Pointer to V4L2 Sub device structure
 * @cfg: Pointer to sub device pad information structure
 *
 * This function is used to initialize the pad format with the default
 * values.
 *
 * Return: 0 on success
 */
static int vip_init_cfg(struct v4l2_subdev *sd,
			      struct v4l2_subdev_state *sd_state)
{
	struct vip_state *vip = to_vipstate(sd);
	struct v4l2_mbus_framefmt *format;
	unsigned int i;

	mutex_lock(&vip->lock);
	for (i = 0; i < VIP_MEDIA_PADS; i++) {
		format = v4l2_subdev_get_try_format(sd, sd_state, i);
		*format = vip->pad_format[i];
	}
	mutex_unlock(&vip->lock);

	return 0;
}

static const u32 vip_src_pad_fmts[] = {MEDIA_BUS_FMT_UYVY8_1X16, MEDIA_BUS_FMT_VYYUYY8_1X24, MEDIA_BUS_FMT_RBG888_1X24};

/*
 * vip_enum_mbus_code - Handle pixel format enumeration
 * @sd: pointer to v4l2 subdev structure
 * @cfg: V4L2 subdev pad configuration
 * @code: pointer to v4l2_subdev_mbus_code_enum structure
 *
 * Return: -EINVAL or zero on success
 */
static int vip_enum_mbus_code(struct v4l2_subdev *sd,
				    struct v4l2_subdev_state *sd_state,
				    struct v4l2_subdev_mbus_code_enum *code)
{
	struct vip_state *vip = to_vipstate(sd);

	if (code->pad == VIP_PAD_SINK && code->index == 0) {
		code->code = vip->pad_format[code->pad].code;
	} else if (code->pad == VIP_PAD_SOURCE && code->index < ARRAY_SIZE(vip_src_pad_fmts)) {
		code->code = vip_src_pad_fmts[code->index];
	} else {
		return -EINVAL;
	}

	return 0;
}

static int vip_enum_frame_size(struct v4l2_subdev *sd,
			       struct v4l2_subdev_state *state,
			       struct v4l2_subdev_frame_size_enum *fse)
{
	struct vip_state *vip = to_vipstate(sd);

	if (fse->index > 0 || fse->pad >= VIP_MEDIA_PADS) {
		return -EINVAL;
	}

	if (fse->pad == VIP_PAD_SINK && fse->index == 0) {
		fse->min_width  = vip->pad_format[fse->pad].width;
		fse->min_height = vip->pad_format[fse->pad].height;
		fse->max_width  = vip->pad_format[fse->pad].width;
		fse->max_height = vip->pad_format[fse->pad].height;
	} else if (fse->pad == VIP_PAD_SOURCE && fse->index == 0) {
		fse->min_width  = vip->pad_format[VIP_PAD_SINK].width / 4 / 4 * 4;
		fse->min_height = vip->pad_format[VIP_PAD_SINK].height / 4 / 4 * 4;
		fse->max_width  = vip->pad_format[VIP_PAD_SINK].width;
		fse->max_height = vip->pad_format[VIP_PAD_SINK].height;
	} else {
		return -EINVAL;
	}

	return 0;
}

static struct v4l2_mbus_framefmt *
__vip_get_pad_format(struct vip_state *vip,
			   struct v4l2_subdev_state *sd_state,
			   unsigned int pad, u32 which)
{
	struct v4l2_mbus_framefmt *get_fmt;

	switch (which) {
	case V4L2_SUBDEV_FORMAT_TRY:
		get_fmt = v4l2_subdev_get_try_format(&vip->subdev,
						     sd_state, pad);
		break;
	case V4L2_SUBDEV_FORMAT_ACTIVE:
		get_fmt = &vip->pad_format[pad];
		break;
	default:
		get_fmt = NULL;
		break;
	}

	return get_fmt;
}

/**
 * vip_get_format - Get the pad format
 * @sd: Pointer to V4L2 Sub device structure
 * @cfg: Pointer to sub device pad information structure
 * @fmt: Pointer to pad level media bus format
 *
 * This function is used to get the pad format information.
 *
 * Return: 0 on success
 */
static int vip_get_format(struct v4l2_subdev *sd,
				struct v4l2_subdev_state *sd_state,
				struct v4l2_subdev_format *fmt)
{
	struct vip_state *vip = to_vipstate(sd);
	struct v4l2_mbus_framefmt *get_fmt;
	int ret = 0;

	mutex_lock(&vip->lock);

	get_fmt = __vip_get_pad_format(vip, sd_state, fmt->pad,
					     fmt->which);
	if (!get_fmt) {
		ret = -EINVAL;
		goto unlock_get_format;
	}

	fmt->format = *get_fmt;

unlock_get_format:
	mutex_unlock(&vip->lock);

	return ret;
}

static int vip_set_format(struct v4l2_subdev *sd,
				struct v4l2_subdev_state *sd_state,
				struct v4l2_subdev_format *fmt)
{
	struct vip_state *vip = to_vipstate(sd);
	struct v4l2_mbus_framefmt *__format;
	int ret = 0, index = 0;

	mutex_lock(&vip->lock);

	__format = __vip_get_pad_format(vip, sd_state,
					      fmt->pad, fmt->which);
	if (!__format) {
		ret = -EINVAL;
		goto unlock_set_format;
	}

	/* only source pad format can be updated */
	if (fmt->pad != VIP_PAD_SOURCE) {
		fmt->format = *__format;
		goto unlock_set_format;
	}

	for (index = 0; index < ARRAY_SIZE(vip_src_pad_fmts); index ++) {
		if (fmt->format.code == vip_src_pad_fmts[index]) {
			__format->code = fmt->format.code;
			break;
		}
	}
	if (fmt->format.width <= vip->pad_format[VIP_PAD_SINK].width && fmt->format.height <= vip->pad_format[VIP_PAD_SINK].height) {
		__format->width  = fmt->format.width;
		__format->height = fmt->format.height;
	}
	fmt->format = *__format;

unlock_set_format:
	mutex_unlock(&vip->lock);

	return ret;
}

/* -----------------------------------------------------------------------------
 * Media Operations
 */

static const struct media_entity_operations vip_media_ops = {
	.link_validate = v4l2_subdev_link_validate
};

static const struct v4l2_subdev_core_ops vip_core_ops = {
	.log_status = vip_log_status,
	.subscribe_event = vip_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static const struct v4l2_subdev_video_ops vip_video_ops = {
	.s_stream = vip_s_stream,
};

static const struct v4l2_subdev_pad_ops vip_pad_ops = {
	.init_cfg = vip_init_cfg,
	.enum_mbus_code = vip_enum_mbus_code,
	.enum_frame_size = vip_enum_frame_size,
	.get_fmt = vip_get_format,
	.set_fmt = vip_set_format,
	.link_validate = v4l2_subdev_link_validate_default,
};

static const struct v4l2_subdev_ops vip_ops = {
	.core = &vip_core_ops,
	.video = &vip_video_ops,
	.pad = &vip_pad_ops
};

static int vip_get_hw_format(struct vip_state *vip)
{
	struct device *dev = vip->dev;
	struct v4l2_mbus_framefmt *format = NULL;

	u32 width  = vip_read(vip, VIP_REG_WIDTH);
	u32 height = vip_read(vip, VIP_REG_HEIGHT);
	u32 bits   = vip_read(vip, VIP_REG_BITS);
	if (width < 1 || height < 1 || (bits != 8 && bits != 10 && bits != 12)) {
		dev_err(dev, "Invalid HW formats. Resolution %u x %u, YUV%u",
			width, height, bits);
		return -EINVAL;
	}

	dev_info(dev, "VIP HW formats. Resolution %u x %u, YUV%u",
		width, height, bits);
	
	vip->bits = bits;
	format = &vip->pad_format[VIP_PAD_SINK];

	switch (bits) {
		case 8:   format->code = MEDIA_BUS_FMT_YUV8_1X24;  break;
		case 10:  format->code = MEDIA_BUS_FMT_YUV10_1X30; break;
		default:  format->code = MEDIA_BUS_FMT_YUV12_1X36; break;
	}

	format->field      = V4L2_FIELD_NONE;
	format->colorspace = V4L2_COLORSPACE_SRGB;
	format->width      = width;
	format->height     = height;

	format = &vip->pad_format[VIP_PAD_SOURCE];

	*format = vip->pad_format[VIP_PAD_SINK];
	format->code = MEDIA_BUS_FMT_UYVY8_1X16;

	return 0;
}

static int vip_initialize_hw(struct vip_state *vip)
{
	//u32 width  = vip->pad_format[VIP_PAD_SINK].width;
	//u32 height = vip->pad_format[VIP_PAD_SINK].height;
	//u32 bits   = vip->bits;
	unsigned int vip_top_en = 0;

	vip_write(vip, VIP_REG_RESET, 1);
	vip_write(vip, VIP_REG_INT_MASK, ~0U);

	//vip_top_en |= VIP_REG_TOP_EN_BIT_HIST_EQU_EN;
	//vip_top_en |= VIP_REG_TOP_EN_BIT_SOBEL_EN;
	//vip_top_en |= VIP_REG_TOP_EN_BIT_YUV2RGB_EN;
	//vip_top_en |= VIP_REG_TOP_EN_BIT_CROP_EN;
	//vip_top_en |= VIP_REG_TOP_EN_BIT_OSD_EN;
	//vip_top_en |= VIP_REG_TOP_EN_BIT_DSCALE_EN;
	vip_top_en |= VIP_REG_TOP_EN_BIT_YUV444TO422_EN;
	vip_write(vip, VIP_REG_TOP_EN, vip_top_en);

	return 0;
}

static int vip_probe(struct platform_device *pdev)
{
	struct v4l2_subdev *subdev;
	struct vip_state *vip;
	int num_clks = ARRAY_SIZE(vip_clks);
	struct device *dev = &pdev->dev;
	int irq, ret;

	vip = devm_kzalloc(dev, sizeof(*vip), GFP_KERNEL);
	if (!vip) {
		dev_err(dev, "No memory for vip");
		return -ENOMEM;
	}

	vip->dev = dev;

	vip->clks = devm_kmemdup(dev, vip_clks, sizeof(vip_clks), GFP_KERNEL);
	if (!vip->clks) {
		dev_err(dev, "No memory for vip clks");
		return -ENOMEM;
	}

	vip->iomem = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(vip->iomem)) {
		dev_err(dev, "No iomem resource in DT");
		return PTR_ERR(vip->iomem);
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "No irq resource in DT");
		return irq;
	}

	ret = devm_request_threaded_irq(dev, irq, NULL,
					vip_irq_handler, IRQF_ONESHOT,
					dev_name(dev), vip);
	if (ret) {
		dev_err(dev, "Err = %d Interrupt handler reg failed!\n", ret);
		return ret;
	}

	ret = devm_clk_bulk_get(dev, num_clks, vip->clks);
	if (ret) {
		dev_err(dev, "could not get clks");
		return ret;
	}

	/* TODO: Enable/disable clocks at stream on/off time. */
	ret = clk_bulk_prepare_enable(num_clks, vip->clks);
	if (ret) {
		dev_err(dev, "could not prepare enable clks");
		return ret;
	}

	mutex_init(&vip->lock);

	/* Initialize V4L2 subdevice and media entity */
	vip->pads[VIP_PAD_SINK].flags = MEDIA_PAD_FL_SINK;
	vip->pads[VIP_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;

	/* Initialize the default format */
	ret = vip_get_hw_format(vip);
	if (ret < 0) {
		goto error;
	}

	/* Initialize the vip hardware */
	ret = vip_initialize_hw(vip);
	if (ret < 0) {
		goto error;
	}

	/* Initialize V4L2 subdevice and media entity */
	subdev = &vip->subdev;
	v4l2_subdev_init(subdev, &vip_ops);
	subdev->dev = dev;
	strscpy(subdev->name, dev_name(dev), sizeof(subdev->name));
	subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS | V4L2_SUBDEV_FL_HAS_DEVNODE;
	subdev->entity.ops = &vip_media_ops;
	v4l2_set_subdevdata(subdev, vip);

	ret = media_entity_pads_init(&subdev->entity, VIP_MEDIA_PADS,
				     vip->pads);
	if (ret < 0) {
		dev_err(dev, "init media entity pads fail");
		goto error;
	}

	platform_set_drvdata(pdev, vip);

	ret = v4l2_async_register_subdev(subdev);
	if (ret < 0) {
		dev_err(dev, "failed to register subdev\n");
		goto error;
	}

	dev_info(dev, "xil-vip driver probed!");

	return 0;
error:
	media_entity_cleanup(&subdev->entity);
	mutex_destroy(&vip->lock);
	clk_bulk_disable_unprepare(num_clks, vip->clks);
	return ret;
}

static int vip_remove(struct platform_device *pdev)
{
	struct vip_state *vip = platform_get_drvdata(pdev);
	struct v4l2_subdev *subdev = &vip->subdev;
	int num_clks = ARRAY_SIZE(vip_clks);

	v4l2_async_unregister_subdev(subdev);
	media_entity_cleanup(&subdev->entity);
	mutex_destroy(&vip->lock);
	clk_bulk_disable_unprepare(num_clks, vip->clks);

	return 0;
}

static const struct of_device_id vip_of_id_table[] = {
	{ .compatible = "xlnx,xil-vip-1.0", },
	{ }
};
MODULE_DEVICE_TABLE(of, vip_of_id_table);

static struct platform_driver vip_driver = {
	.driver = {
		.name		= "xil-vip",
		.of_match_table	= vip_of_id_table,
	},
	.probe			= vip_probe,
	.remove			= vip_remove,
};

module_platform_driver(vip_driver);

MODULE_AUTHOR("xinquan bian <544177215@qq.com>");
MODULE_DESCRIPTION("Xil VIP Driver");
MODULE_LICENSE("GPL v2");
