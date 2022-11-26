// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for MIPI Rx to Video
 *
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
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

#define MIPI_RX_REG_RESET                (0 * 4)    
#define MIPI_RX_REG_WIDTH                (1 * 4)    
#define MIPI_RX_REG_HEIGHT               (2 * 4)    
#define MIPI_RX_REG_FRAME_CNT            (3 * 4)    
#define MIPI_RX_REG_DI                   (4 * 4)    
#define MIPI_RX_REG_STATUS               (5 * 4)    
#define MIPI_RX_REG_INT_STATUS           (6 * 4)    
#define MIPI_RX_REG_INT_MASK             (7 * 4)    

#define MIPI_RX_REG_STATUS_BIT_LOCKED            (1<<0)

#define MIPI_RX_REG_INT_STATUS_BIT_FRAME_START   (1<<0)
#define MIPI_RX_REG_INT_STATUS_BIT_FRAME_DONE    (1<<1)

#define MIPI_RX_REG_INT_MASK_BIT_FRAME_START     (1<<0)
#define MIPI_RX_REG_INT_MASK_BIT_FRAME_DONE      (1<<1)

////////////////////////////////////

#define MIPI_RX_PAD_SINK		0
#define MIPI_RX_PAD_SOURCE		1
#define MIPI_RX_MEDIA_PADS		2


/**
 * struct mipirx_state - CSI-2 Rx device structure
 * @subdev: The v4l2 subdev structure
 * @format: Active V4L2 formats on each pad
 * @default_format: Default V4L2 format
 * @dev: Platform structure
 * @rst_gpio: reset to video_aresetn
 * @aclk: s_axi clock
 * @iomem: Base address of subsystem
 * @lock: mutex for accessing this structure
 * @pads: media pads
 * @streaming: Flag for storing streaming state
 *
 * This structure contains the device driver related parameters
 */
struct mipirx_state {
	struct v4l2_subdev subdev;
	struct v4l2_mbus_framefmt format;
	struct v4l2_mbus_framefmt default_format;
	struct device *dev;
	struct gpio_desc *rst_gpio;
	struct clk	*aclk;
	void __iomem *iomem;
	/* used to protect access to this struct */
	struct mutex lock;
	struct media_pad pads[MIPI_RX_MEDIA_PADS];
	bool streaming;
};

static inline struct mipirx_state *
to_mipirxstate(struct v4l2_subdev *subdev)
{
	return container_of(subdev, struct mipirx_state, subdev);
}

/*
 * Register related operations
 */
static inline u32 mipirx_read(struct mipirx_state *mipirx, u32 addr)
{
	return ioread32(mipirx->iomem + addr);
}

static inline void mipirx_write(struct mipirx_state *mipirx, u32 addr,
				   u32 value)
{
	iowrite32(value, mipirx->iomem + addr);
}

static void mipirx_hard_reset(struct mipirx_state *state)
{
	if (!state->rst_gpio) {
		return;
	}

	/* minimum of 40 dphy_clk_200M cycles */
	gpiod_set_value_cansleep(state->rst_gpio, 1);
	usleep_range(1, 2);
	gpiod_set_value_cansleep(state->rst_gpio, 0);
}

/**
 * mipirx_log_status - Logs the status of the CSI-2 Receiver
 * @sd: Pointer to V4L2 subdevice structure
 *
 * This function prints the current status of MIPI Rx to Video IP
 *
 * Return: 0 on success
 */
static int mipirx_log_status(struct v4l2_subdev *sd)
{
	struct mipirx_state *mipirx = to_mipirxstate(sd);
	struct device *dev = mipirx->dev;
	u32 width, height, frame_cnt, di, status;

	mutex_lock(&mipirx->lock);

	width     = mipirx_read(mipirx, MIPI_RX_REG_WIDTH);
	height    = mipirx_read(mipirx, MIPI_RX_REG_HEIGHT);
	di        = mipirx_read(mipirx, MIPI_RX_REG_DI);
	frame_cnt = mipirx_read(mipirx, MIPI_RX_REG_FRAME_CNT);
	status    = mipirx_read(mipirx, MIPI_RX_REG_STATUS);
	dev_info(dev, "MIPI Rx %u x %u, DI %02X, frame_cnt %u, status %04X",
			width, height, di, frame_cnt, status);

	mutex_unlock(&mipirx->lock);

	return 0;
}

static int mipirx_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			       struct v4l2_event_subscription *sub)
{
	if (sub->type != V4L2_EVENT_FRAME_SYNC)
		return -EINVAL;

	/* V4L2_EVENT_FRAME_SYNC doesn't require an id, so zero should be set */
	if (sub->id != 0)
		return -EINVAL;

	return v4l2_event_subscribe(fh, sub, 0, NULL);
}

static int mipirx_start_stream(struct mipirx_state *mipirx)
{
	mipirx_write(mipirx, MIPI_RX_REG_INT_STATUS, 0);
	mipirx_write(mipirx, MIPI_RX_REG_INT_MASK, ~MIPI_RX_REG_INT_MASK_BIT_FRAME_START);
	mipirx_write(mipirx, MIPI_RX_REG_RESET, 0);

	mipirx->streaming = true;

	return 0;
}

static void mipirx_stop_stream(struct mipirx_state *mipirx)
{
	mipirx_write(mipirx, MIPI_RX_REG_RESET, 1);
	mipirx_write(mipirx, MIPI_RX_REG_INT_MASK, ~0);
	mipirx_write(mipirx, MIPI_RX_REG_INT_STATUS, 0);

	mipirx->streaming = false;
}

static void mipirx_queue_event_frame_sync(struct mipirx_state *state, u32 frame_seq)
{
	struct v4l2_event event = {
		.type = V4L2_EVENT_FRAME_SYNC,
	};
	event.u.frame_sync.frame_sequence = frame_seq;

	v4l2_event_queue(state->subdev.devnode, &event);
}

/**
 * mipirx_irq_handler - Interrupt handler for CSI-2
 * @irq: IRQ number
 * @data: Pointer to device state
 *
 * In the interrupt handler, a list of event counters are updated for
 * corresponding interrupts. This is useful to get status / debug.
 *
 * Return: IRQ_HANDLED after handling interrupts
 */
static irqreturn_t mipirx_irq_handler(int irq, void *data)
{
	struct mipirx_state *state = (struct mipirx_state *)data;
	//struct device *dev = state->dev;
	u32 status;

	status = mipirx_read(state, MIPI_RX_REG_INT_STATUS);
	mipirx_write(state, MIPI_RX_REG_INT_STATUS, 0);

	if (status & MIPI_RX_REG_INT_STATUS_BIT_FRAME_START) {
		u32 frame_seq = mipirx_read(state, MIPI_RX_REG_FRAME_CNT);
		//dev_info(dev, "IRQ frame start, seq %u", frame_seq);
		mipirx_queue_event_frame_sync(state, frame_seq);
	}

	if (status & MIPI_RX_REG_INT_STATUS_BIT_FRAME_DONE) {
		//XXX
	}

	return IRQ_HANDLED;
}

/**
 * mipirx_s_stream - It is used to start/stop the streaming.
 * @sd: V4L2 Sub device
 * @enable: Flag (True / False)
 *
 * This function controls the start or stop of streaming for the
 * MIPI CSI-2 Rx to Video.
 *
 * Return: 0 on success, errors otherwise
 */
static int mipirx_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct mipirx_state *mipirx = to_mipirxstate(sd);
	int ret = 0;

	mutex_lock(&mipirx->lock);

	if (enable == mipirx->streaming) {
		goto stream_done;
	}

	mipirx_hard_reset(mipirx);

	if (enable) {
		ret = mipirx_start_stream(mipirx);
	} else {
		mipirx_stop_stream(mipirx);
	}

stream_done:
	mutex_unlock(&mipirx->lock);
	return ret;
}

static struct v4l2_mbus_framefmt *
__mipirx_get_pad_format(struct mipirx_state *mipirx,
			   struct v4l2_subdev_state *sd_state,
			   unsigned int pad, u32 which)
{
	struct v4l2_mbus_framefmt *get_fmt;

	switch (which) {
	case V4L2_SUBDEV_FORMAT_TRY:
		get_fmt = v4l2_subdev_get_try_format(&mipirx->subdev,
						     sd_state, pad);
		break;
	case V4L2_SUBDEV_FORMAT_ACTIVE:
		get_fmt = &mipirx->format;
		break;
	default:
		get_fmt = NULL;
		break;
	}

	return get_fmt;
}

/**
 * mipirx_init_cfg - Initialise the pad format config to default
 * @sd: Pointer to V4L2 Sub device structure
 * @cfg: Pointer to sub device pad information structure
 *
 * This function is used to initialize the pad format with the default
 * values.
 *
 * Return: 0 on success
 */
static int mipirx_init_cfg(struct v4l2_subdev *sd,
			      struct v4l2_subdev_state *sd_state)
{
	struct mipirx_state *mipirx = to_mipirxstate(sd);
	struct v4l2_mbus_framefmt *format;
	unsigned int i;

	mutex_lock(&mipirx->lock);
	for (i = 0; i < MIPI_RX_MEDIA_PADS; i++) {
		format = v4l2_subdev_get_try_format(sd, sd_state, i);
		*format = mipirx->default_format;
	}
	mutex_unlock(&mipirx->lock);

	return 0;
}

/**
 * mipirx_get_format - Get the pad format
 * @sd: Pointer to V4L2 Sub device structure
 * @cfg: Pointer to sub device pad information structure
 * @fmt: Pointer to pad level media bus format
 *
 * This function is used to get the pad format information.
 *
 * Return: 0 on success
 */
static int mipirx_get_format(struct v4l2_subdev *sd,
				struct v4l2_subdev_state *sd_state,
				struct v4l2_subdev_format *fmt)
{
	struct mipirx_state *mipirx = to_mipirxstate(sd);
	struct v4l2_mbus_framefmt *get_fmt;
	int ret = 0;

	mutex_lock(&mipirx->lock);

	get_fmt = __mipirx_get_pad_format(mipirx, sd_state, fmt->pad,
					     fmt->which);
	if (!get_fmt) {
		ret = -EINVAL;
		goto unlock_get_format;
	}

	fmt->format = *get_fmt;

unlock_get_format:
	mutex_unlock(&mipirx->lock);

	return ret;
}

/**
 * mipirx_set_format - This is used to set the pad format
 * @sd: Pointer to V4L2 Sub device structure
 * @cfg: Pointer to sub device pad information structure
 * @fmt: Pointer to pad level media bus format
 *
 * This function is used to set the pad format. Since the pad format is fixed
 * in hardware, it can't be modified on run time. So when a format set is
 * requested by application, all parameters except the format type is saved
 * for the pad and the original pad format is sent back to the application.
 *
 * Return: 0 on success
 */
static int mipirx_set_format(struct v4l2_subdev *sd,
				struct v4l2_subdev_state *sd_state,
				struct v4l2_subdev_format *fmt)
{
	struct mipirx_state *mipirx = to_mipirxstate(sd);
	struct v4l2_mbus_framefmt *__format;
	int ret = 0;

	mutex_lock(&mipirx->lock);

	/*
	 * Only the format->code parameter matters for CSI as the
	 * CSI format cannot be changed at runtime.
	 * Ensure that format to set is copied to over to CSI pad format
	 */
	__format = __mipirx_get_pad_format(mipirx, sd_state,
					      fmt->pad, fmt->which);
	if (!__format) {
		ret = -EINVAL;
		goto unlock_set_format;
	}

	/* only sink pad format can be updated */
	if (fmt->pad == MIPI_RX_PAD_SOURCE) {
		fmt->format = *__format;
		goto unlock_set_format;
	}

	*__format = fmt->format;

unlock_set_format:
	mutex_unlock(&mipirx->lock);

	return ret;
}

/*
 * mipirx_enum_mbus_code - Handle pixel format enumeration
 * @sd: pointer to v4l2 subdev structure
 * @cfg: V4L2 subdev pad configuration
 * @code: pointer to v4l2_subdev_mbus_code_enum structure
 *
 * Return: -EINVAL or zero on success
 */
static int mipirx_enum_mbus_code(struct v4l2_subdev *sd,
				    struct v4l2_subdev_state *sd_state,
				    struct v4l2_subdev_mbus_code_enum *code)
{
	//struct mipirx_state *state = to_mipirxstate(sd);

	const u32 support_codes[] = {
		MEDIA_BUS_FMT_SBGGR8_1X8,
		MEDIA_BUS_FMT_SGBRG8_1X8,
		MEDIA_BUS_FMT_SGRBG8_1X8,
		MEDIA_BUS_FMT_SRGGB8_1X8,
		MEDIA_BUS_FMT_SBGGR10_1X10,
		MEDIA_BUS_FMT_SGBRG10_1X10,
		MEDIA_BUS_FMT_SGRBG10_1X10,
		MEDIA_BUS_FMT_SRGGB10_1X10
	};

	if (code->index >= ARRAY_SIZE(support_codes)) {
		return -EINVAL;
	}

	code->code = support_codes[code->index];

	return 0;
}

/* -----------------------------------------------------------------------------
 * Media Operations
 */

static const struct media_entity_operations mipirx_media_ops = {
	.link_validate = v4l2_subdev_link_validate
};

static const struct v4l2_subdev_core_ops mipirx_core_ops = {
	.log_status = mipirx_log_status,
	.subscribe_event = mipirx_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static const struct v4l2_subdev_video_ops mipirx_video_ops = {
	.s_stream = mipirx_s_stream
};

static const struct v4l2_subdev_pad_ops mipirx_pad_ops = {
	.init_cfg = mipirx_init_cfg,
	.get_fmt = mipirx_get_format,
	.set_fmt = mipirx_set_format,
	.enum_mbus_code = mipirx_enum_mbus_code,
	.link_validate = v4l2_subdev_link_validate_default,
};

static const struct v4l2_subdev_ops mipirx_ops = {
	.core = &mipirx_core_ops,
	.video = &mipirx_video_ops,
	.pad = &mipirx_pad_ops
};

static int mipirx_probe(struct platform_device *pdev)
{
	struct v4l2_subdev *subdev;
	struct mipirx_state *mipirx;
	struct device *dev = &pdev->dev;
	int irq, ret;

	mipirx = devm_kzalloc(dev, sizeof(*mipirx), GFP_KERNEL);
	if (!mipirx) {
		dev_err(dev, "No memory for mipirx");
		return -ENOMEM;
	}

	mipirx->dev = dev;

	/* Reset GPIO */
	mipirx->rst_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(mipirx->rst_gpio)) {
		dev_err(dev, "Video Reset GPIO not setup in DT, ignore it");
		mipirx->rst_gpio = NULL;
	}

	mipirx->iomem = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(mipirx->iomem)) {
		dev_err(dev, "No iomem resource in DT");
		return PTR_ERR(mipirx->iomem);
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "No irq resource in DT");
		return irq;
	}

	ret = devm_request_threaded_irq(dev, irq, NULL,
					mipirx_irq_handler, IRQF_ONESHOT,
					dev_name(dev), mipirx);
	if (ret) {
		dev_err(dev, "Err = %d Interrupt handler reg failed!\n", ret);
		return ret;
	}

	mipirx->aclk = devm_clk_get(dev, "s00_axi_aclk");
	if (IS_ERR(mipirx->aclk)) {
		dev_err(dev, "could not get s00_axi_aclk");
		return PTR_ERR(mipirx->aclk);
	}

	/* TODO: Enable/disable clocks at stream on/off time. */
	ret = clk_prepare_enable(mipirx->aclk);
	if (ret) {
		dev_err(dev, "could not prepare enable aclk");
		return ret;
	}

	mutex_init(&mipirx->lock);

	mipirx_hard_reset(mipirx);
	mipirx_write(mipirx, MIPI_RX_REG_RESET, 1);

	/* Initialize V4L2 subdevice and media entity */
	mipirx->pads[MIPI_RX_PAD_SINK].flags = MEDIA_PAD_FL_SINK;
	mipirx->pads[MIPI_RX_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;

	/* Initialize the default format */
	mipirx->default_format.code = MEDIA_BUS_FMT_SRGGB10_1X10;
	mipirx->default_format.field = V4L2_FIELD_NONE;
	mipirx->default_format.colorspace = V4L2_COLORSPACE_SRGB;
	mipirx->default_format.width = 1920;
	mipirx->default_format.height = 1080;
	mipirx->format = mipirx->default_format;

	/* Initialize V4L2 subdevice and media entity */
	subdev = &mipirx->subdev;
	v4l2_subdev_init(subdev, &mipirx_ops);
	subdev->dev = dev;
	strscpy(subdev->name, dev_name(dev), sizeof(subdev->name));
	subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS | V4L2_SUBDEV_FL_HAS_DEVNODE;
	subdev->entity.ops = &mipirx_media_ops;
	v4l2_set_subdevdata(subdev, mipirx);

	ret = media_entity_pads_init(&subdev->entity, MIPI_RX_MEDIA_PADS,
				     mipirx->pads);
	if (ret < 0) {
		dev_err(dev, "init media entity pads fail");
		goto error;
	}

	platform_set_drvdata(pdev, mipirx);

	ret = v4l2_async_register_subdev(subdev);
	if (ret < 0) {
		dev_err(dev, "failed to register subdev\n");
		goto error;
	}

	dev_info(dev, "mipi-rx-to-video driver probed!");

	return 0;
error:
	media_entity_cleanup(&subdev->entity);
	mutex_destroy(&mipirx->lock);
	clk_disable_unprepare(mipirx->aclk);
	return ret;
}

static int mipirx_remove(struct platform_device *pdev)
{
	struct mipirx_state *mipirx = platform_get_drvdata(pdev);
	struct v4l2_subdev *subdev = &mipirx->subdev;

	v4l2_async_unregister_subdev(subdev);
	media_entity_cleanup(&subdev->entity);
	mutex_destroy(&mipirx->lock);
	clk_disable_unprepare(mipirx->aclk);

	return 0;
}

static const struct of_device_id mipirx_of_id_table[] = {
	{ .compatible = "xlnx,mipi-rx-to-video-1.0", },
	{ }
};
MODULE_DEVICE_TABLE(of, mipirx_of_id_table);

static struct platform_driver mipirx_driver = {
	.driver = {
		.name		= "mipi-rx-to-video",
		.of_match_table	= mipirx_of_id_table,
	},
	.probe			= mipirx_probe,
	.remove			= mipirx_remove,
};

module_platform_driver(mipirx_driver);

MODULE_AUTHOR("xinquan bian <544177215@qq.com>");
MODULE_DESCRIPTION("MIPI CSI-2 Rx to video Driver");
MODULE_LICENSE("GPL v2");
