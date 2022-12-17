// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Xil ISP Lite IP
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
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-vmalloc.h>
#include <media/v4l2-ctrls.h>
#include <linux/xil-isp-lite.h>

////////////////////////////////////////////////////////////////////////////
/* Register register map */

#define ISP_REG_RESET                   (0  * 4)
#define ISP_REG_WIDTH                   (1  * 4)
#define ISP_REG_HEIGHT                  (2  * 4)
#define ISP_REG_BITS                    (3  * 4)
#define ISP_REG_BAYER                   (4  * 4)
#define ISP_REG_TOP_EN                  (5  * 4)
#define ISP_REG_DPC_THRESHOLD           (6  * 4)
#define ISP_REG_BLC_R                   (7  * 4)
#define ISP_REG_BLC_GR                  (8  * 4)
#define ISP_REG_BLC_GB                  (9  * 4)
#define ISP_REG_BLC_B                   (10 * 4)
#define ISP_REG_NR_LEVEL                (11 * 4)
#define ISP_REG_DGAIN_GAIN              (12 * 4)
#define ISP_REG_DGAIN_OFFSET            (13 * 4)
#define ISP_REG_WB_RGAIN                (14 * 4)
#define ISP_REG_WB_GGAIN                (15 * 4)
#define ISP_REG_WB_BGAIN                (16 * 4)
#define ISP_REG_CCM_RR                  (17 * 4)
#define ISP_REG_CCM_RG                  (18 * 4)
#define ISP_REG_CCM_RB                  (19 * 4)
#define ISP_REG_CCM_GR                  (20 * 4)
#define ISP_REG_CCM_GG                  (21 * 4)
#define ISP_REG_CCM_GB                  (22 * 4)
#define ISP_REG_CCM_BR                  (23 * 4)
#define ISP_REG_CCM_BG                  (24 * 4)
#define ISP_REG_CCM_BB                  (25 * 4)
#define ISP_REG_STAT_AE_RECT_X          (26 * 4)
#define ISP_REG_STAT_AE_RECT_Y          (27 * 4)
#define ISP_REG_STAT_AE_RECT_W          (28 * 4)
#define ISP_REG_STAT_AE_RECT_H          (29 * 4)
#define ISP_REG_STAT_AE_PIX_CNT_L       (30 * 4)
#define ISP_REG_STAT_AE_PIX_CNT_H       (31 * 4)
#define ISP_REG_STAT_AE_SUM_L           (32 * 4)
#define ISP_REG_STAT_AE_SUM_H           (33 * 4)
#define ISP_REG_STAT_AWB_MIN            (34 * 4)
#define ISP_REG_STAT_AWB_MAX            (35 * 4)
#define ISP_REG_STAT_AWB_PIX_CNT_L      (36 * 4)
#define ISP_REG_STAT_AWB_PIX_CNT_H      (37 * 4)
#define ISP_REG_STAT_AWB_SUM_R_L        (38 * 4)
#define ISP_REG_STAT_AWB_SUM_R_H        (39 * 4)
#define ISP_REG_STAT_AWB_SUM_G_L        (40 * 4)
#define ISP_REG_STAT_AWB_SUM_G_H        (41 * 4)
#define ISP_REG_STAT_AWB_SUM_B_L        (42 * 4)
#define ISP_REG_STAT_AWB_SUM_B_H        (43 * 4)
#define ISP_REG_INT_STATUS              (44 * 4)
#define ISP_REG_INT_MASK                (45 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_0     (46 * 4) //0:[4:0], 1:[12:8], 2:[20:16], 3:[28:24]
#define ISP_REG_2DNR_SPACE_WEIGHT_4     (47 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_8     (48 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_12    (49 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_16    (50 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_20    (51 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_24    (52 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_28    (53 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_32    (54 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_36    (55 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_40    (56 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_44    (57 * 4)
#define ISP_REG_2DNR_SPACE_WEIGHT_48    (58 * 4) //48:[4:0]
#define ISP_REG_2DNR_COLOR_CURVE_0      (59 * 4) //x0:[15:0] y0:[20:16]
#define ISP_REG_2DNR_COLOR_CURVE_1      (60 * 4)
#define ISP_REG_2DNR_COLOR_CURVE_2      (61 * 4)
#define ISP_REG_2DNR_COLOR_CURVE_3      (62 * 4)
#define ISP_REG_2DNR_COLOR_CURVE_4      (63 * 4)
#define ISP_REG_2DNR_COLOR_CURVE_5      (64 * 4)
#define ISP_REG_2DNR_COLOR_CURVE_6      (65 * 4)
#define ISP_REG_2DNR_COLOR_CURVE_7      (66 * 4)
#define ISP_REG_2DNR_COLOR_CURVE_8      (67 * 4)


#define ISP_REG_STAT_AE_HIST_ADDR     	(1*1024*4)
#define ISP_REG_STAT_AE_HIST_SIZE     	(4*256*4)
#define ISP_REG_STAT_AWB_HIST_ADDR    	(2*1024*4)
#define ISP_REG_STAT_AWB_HIST_SIZE      (3*256*4)
#define ISP_REG_GAMMA_TABLE_ADDR      	(3*1024*4)
#define ISP_REG_GAMMA_TABLE_SIZE      	(1*64*4)


#define ISP_REG_TOP_EN_BIT_DPC_EN           (1<<0)
#define ISP_REG_TOP_EN_BIT_BLC_EN           (1<<1)
#define ISP_REG_TOP_EN_BIT_BNR_EN           (1<<2)
#define ISP_REG_TOP_EN_BIT_DGAIN_EN         (1<<3)
#define ISP_REG_TOP_EN_BIT_DEMOSIC_EN       (1<<4)
#define ISP_REG_TOP_EN_BIT_WB_EN            (1<<5)
#define ISP_REG_TOP_EN_BIT_CCM_EN           (1<<6)
#define ISP_REG_TOP_EN_BIT_CSC_EN           (1<<7)
#define ISP_REG_TOP_EN_BIT_GAMMA_EN         (1<<8)
#define ISP_REG_TOP_EN_BIT_2DNR_EN          (1<<9)
#define ISP_REG_TOP_EN_BIT_EE_EN            (1<<10)
#define ISP_REG_TOP_EN_BIT_STAT_AE_EN       (1<<11)
#define ISP_REG_TOP_EN_BIT_STAT_AWB_EN      (1<<12)

#define ISP_REG_INT_STATUS_BIT_FRAME_START  (1<<0)
#define ISP_REG_INT_STATUS_BIT_FRAME_DONE   (1<<1)
#define ISP_REG_INT_STATUS_BIT_AE_DONE      (1<<2)
#define ISP_REG_INT_STATUS_BIT_AWB_DONE     (1<<3)

#define ISP_REG_INT_MASK_BIT_FRAME_START    (1<<0)
#define ISP_REG_INT_MASK_BIT_FRAME_DONE     (1<<1)
#define ISP_REG_INT_MASK_BIT_AE_DONE        (1<<2)
#define ISP_REG_INT_MASK_BIT_AWB_DONE       (1<<3)

////////////////////////////////////////////////////////////////////////////

#define ISP_PAD_SINK		0
#define ISP_PAD_SOURCE		1
#define ISP_PAD_SOURCE_2	2
//#define ISP_MEDIA_PADS		2
#define ISP_MEDIA_PADS		3

#define ISP_DRIVER_NAME		"xil-isp-lite"
#define ISP_BUS_NAME		"platform:" ISP_DRIVER_NAME
#define ISP_STAT_DEV_NAME	ISP_DRIVER_NAME "_stat"

struct isp_stat_buffer {
	struct vb2_v4l2_buffer vb;
	struct list_head list_node;
};

struct isp_stat_node {
	struct video_device vdev;
	struct vb2_queue queue;
	struct mutex vlock; /* ioctl serialization mutex */
	struct media_pad pad;
	spinlock_t lock; /* locks the buffers list 'stat' */
	struct list_head buf_list;
};

struct isp_state {
	struct v4l2_subdev subdev;
	struct v4l2_mbus_framefmt pad_format[ISP_MEDIA_PADS];
	struct device *dev;
	struct clk_bulk_data *clks;
	void __iomem *iomem;
	u32 bits;
	/* used to protect access to this struct */
	struct mutex lock;
	struct media_pad pads[ISP_MEDIA_PADS];
	bool streaming;
	u32 frame_sequence;
	u32 int_status;

	struct v4l2_ctrl_handler config_ctrls;
	struct isp_stat_node    stat_node;
};

static inline struct isp_state *to_ispstate(struct v4l2_subdev *subdev)
{
	return container_of(subdev, struct isp_state, subdev);
}

/*
 * Register related operations
 */
static inline u32 isp_read(struct isp_state *isp, u32 addr)
{
	return ioread32(isp->iomem + addr);
}

static inline void isp_write(struct isp_state *isp, u32 addr,
				   u32 value)
{
	iowrite32(value, isp->iomem + addr);
}


/////////////////////////////////////////////////////////////////////////////
// ISP config/statistics

static int isp_get_top (struct isp_state *isp, struct xil_isp_lite_top *top)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	*(u32*)top = top_en;
	return 0;
}

static int isp_set_top (struct isp_state *isp, const struct xil_isp_lite_top *top)
{
	isp_write(isp, ISP_REG_TOP_EN, *(u32*)top);
	return 0;
}

static int isp_get_dpc (struct isp_state *isp, struct xil_isp_lite_dpc *dpc)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	u32 thresh = isp_read(isp, ISP_REG_DPC_THRESHOLD);
	dpc->enabled   = !!(top_en & ISP_REG_TOP_EN_BIT_DPC_EN);
	dpc->threshold = thresh;
	return 0;
}

static int isp_set_dpc (struct isp_state *isp, const struct xil_isp_lite_dpc *dpc)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = dpc->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_DPC_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_DPC_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	isp_write(isp, ISP_REG_DPC_THRESHOLD, dpc->threshold);
	return 0;
}

static int isp_get_blc (struct isp_state *isp, struct xil_isp_lite_blc *blc)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	blc->enabled   = !!(top_en & ISP_REG_TOP_EN_BIT_BLC_EN);
	blc->black_level_r = isp_read(isp, ISP_REG_BLC_R);
	blc->black_level_gr = isp_read(isp, ISP_REG_BLC_GR);
	blc->black_level_gb = isp_read(isp, ISP_REG_BLC_GB);
	blc->black_level_b = isp_read(isp, ISP_REG_BLC_B);
	return 0;
}

static int isp_set_blc (struct isp_state *isp, const struct xil_isp_lite_blc *blc)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = blc->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_BLC_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_BLC_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	isp_write(isp, ISP_REG_BLC_R, blc->black_level_r);
	isp_write(isp, ISP_REG_BLC_GR, blc->black_level_gr);
	isp_write(isp, ISP_REG_BLC_GB, blc->black_level_gb);
	isp_write(isp, ISP_REG_BLC_B, blc->black_level_b);
	return 0;
}

static int isp_get_bnr (struct isp_state *isp, struct xil_isp_lite_bnr *bnr)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	bnr->enabled = !!(top_en & ISP_REG_TOP_EN_BIT_BNR_EN);
	bnr->nr_level = isp_read(isp, ISP_REG_NR_LEVEL);
	return 0;
}

static int isp_set_bnr (struct isp_state *isp, const struct xil_isp_lite_bnr *bnr)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = bnr->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_BNR_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_BNR_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	isp_write(isp, ISP_REG_NR_LEVEL, bnr->nr_level);
	return 0;
}

static int isp_get_dgain (struct isp_state *isp, struct xil_isp_lite_dgain *dgain)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	dgain->enabled = !!(top_en & ISP_REG_TOP_EN_BIT_DGAIN_EN);
	dgain->gain = isp_read(isp, ISP_REG_DGAIN_GAIN);
	dgain->offset = isp_read(isp, ISP_REG_DGAIN_OFFSET);
	return 0;
}

static int isp_set_dgain (struct isp_state *isp, const struct xil_isp_lite_dgain *dgain)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = dgain->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_DGAIN_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_DGAIN_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	isp_write(isp, ISP_REG_DGAIN_GAIN, dgain->gain);
	isp_write(isp, ISP_REG_DGAIN_OFFSET, dgain->offset);
	return 0;
}

static int isp_get_demosaic (struct isp_state *isp, struct xil_isp_lite_demosaic *demosaic)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	demosaic->enabled   = !!(top_en & ISP_REG_TOP_EN_BIT_DEMOSIC_EN);
	return 0;
}

static int isp_set_demosaic (struct isp_state *isp, const struct xil_isp_lite_demosaic *demosaic)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = demosaic->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_DEMOSIC_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_DEMOSIC_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	return 0;
}

static int isp_get_wb(struct isp_state *isp, struct xil_isp_lite_wb *wb)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	wb->enabled = !!(top_en & ISP_REG_TOP_EN_BIT_WB_EN);
	wb->rgain =  isp_read(isp, ISP_REG_WB_RGAIN);
	wb->ggain =  isp_read(isp, ISP_REG_WB_GGAIN);
	wb->bgain =  isp_read(isp, ISP_REG_WB_BGAIN);
	return 0;
}

static int isp_set_wb(struct isp_state *isp, const struct xil_isp_lite_wb *wb)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = wb->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_WB_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_WB_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	isp_write(isp, ISP_REG_WB_RGAIN, wb->rgain);
	isp_write(isp, ISP_REG_WB_GGAIN, wb->ggain);
	isp_write(isp, ISP_REG_WB_BGAIN, wb->bgain);
	return 0;
}

static int isp_get_ccm(struct isp_state *isp, struct xil_isp_lite_ccm *ccm)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	ccm->enabled = !!(top_en & ISP_REG_TOP_EN_BIT_CCM_EN);
	ccm->matrix[0] = isp_read(isp, ISP_REG_CCM_RR);
	ccm->matrix[1] = isp_read(isp, ISP_REG_CCM_RG);
	ccm->matrix[2] = isp_read(isp, ISP_REG_CCM_RB);
	ccm->matrix[3] = isp_read(isp, ISP_REG_CCM_GR);
	ccm->matrix[4] = isp_read(isp, ISP_REG_CCM_GG);
	ccm->matrix[5] = isp_read(isp, ISP_REG_CCM_GB);
	ccm->matrix[6] = isp_read(isp, ISP_REG_CCM_BR);
	ccm->matrix[7] = isp_read(isp, ISP_REG_CCM_BG);
	ccm->matrix[8] = isp_read(isp, ISP_REG_CCM_BB);
	return 0;
}

static int isp_set_ccm(struct isp_state *isp, const struct xil_isp_lite_ccm *ccm)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = ccm->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_CCM_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_CCM_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	isp_write(isp, ISP_REG_CCM_RR, ccm->matrix[0]);
	isp_write(isp, ISP_REG_CCM_RG, ccm->matrix[1]);
	isp_write(isp, ISP_REG_CCM_RB, ccm->matrix[2]);
	isp_write(isp, ISP_REG_CCM_GR, ccm->matrix[3]);
	isp_write(isp, ISP_REG_CCM_GG, ccm->matrix[4]);
	isp_write(isp, ISP_REG_CCM_GB, ccm->matrix[5]);
	isp_write(isp, ISP_REG_CCM_BR, ccm->matrix[6]);
	isp_write(isp, ISP_REG_CCM_BG, ccm->matrix[7]);
	isp_write(isp, ISP_REG_CCM_BB, ccm->matrix[8]);
	return 0;
}

static int isp_get_csc(struct isp_state *isp, struct xil_isp_lite_csc *csc)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	csc->enabled = !!(top_en & ISP_REG_TOP_EN_BIT_CSC_EN);
	return 0;
}

static int isp_set_csc(struct isp_state *isp, const struct xil_isp_lite_csc *csc)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = csc->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_CSC_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_CSC_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	return 0;
}

static int isp_get_gamma(struct isp_state *isp, struct xil_isp_lite_gamma *gamma)
{
	u32 i;
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	gamma->enabled = !!(top_en & ISP_REG_TOP_EN_BIT_GAMMA_EN);
	for (i = 0; i < ARRAY_SIZE(gamma->gamma_table); i ++) {
		gamma->gamma_table[i] = isp_read(isp, ISP_REG_GAMMA_TABLE_ADDR + i * 4);
	}
	return 0;
}

static int isp_set_gamma(struct isp_state *isp, const struct xil_isp_lite_gamma *gamma)
{
	u32 i;
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = gamma->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_GAMMA_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_GAMMA_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	for (i = 0; i < ARRAY_SIZE(gamma->gamma_table); i ++) {
		isp_write(isp, ISP_REG_GAMMA_TABLE_ADDR + i * 4, gamma->gamma_table[i]);
	}
	return 0;
}

static int isp_get_2dnr(struct isp_state *isp, struct xil_isp_lite_2dnr *nr2d)
{
	u32 i;
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	nr2d->enabled = !!(top_en & ISP_REG_TOP_EN_BIT_2DNR_EN);
	for (i = 0; i < ARRAY_SIZE(nr2d->space_weight); i += 4) {
		u32 data = isp_read(isp, ISP_REG_2DNR_SPACE_WEIGHT_0 + i * 4);
		nr2d->space_weight[i] = (data >> 0) & 0x0ff;
		if (i + 1 < ARRAY_SIZE(nr2d->space_weight))
			nr2d->space_weight[i+1] = (data >> 8) & 0x0ff;
		if (i + 2 < ARRAY_SIZE(nr2d->space_weight))
			nr2d->space_weight[i+2] = (data >> 16) & 0x0ff;
		if (i + 3 < ARRAY_SIZE(nr2d->space_weight))
			nr2d->space_weight[i+3] = (data >> 24) & 0x0ff;
	}
	for (i = 0; i < ARRAY_SIZE(nr2d->color_curve); i++) {
		u32 data = isp_read(isp, ISP_REG_2DNR_COLOR_CURVE_0 + i * 4);
		nr2d->color_curve[i][0] = (data >> (isp->bits-8)) & 0x0ff;
		nr2d->color_curve[i][1] = (data >> 16) & 0x0ff;
	}
	return 0;
}

static int isp_set_2dnr(struct isp_state *isp, const struct xil_isp_lite_2dnr *nr2d)
{
	u32 i;
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = nr2d->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_2DNR_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_2DNR_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	for (i = 0; i < ARRAY_SIZE(nr2d->space_weight); i += 4) {
		u32 data = nr2d->space_weight[i];
		if (i + 1 < ARRAY_SIZE(nr2d->space_weight))
			data |= nr2d->space_weight[i+1] << 8;
		if (i + 2 < ARRAY_SIZE(nr2d->space_weight))
			data |= nr2d->space_weight[i+2] << 16;
		if (i + 3 < ARRAY_SIZE(nr2d->space_weight))
			data |= nr2d->space_weight[i+3] << 24;
		isp_write(isp, ISP_REG_2DNR_SPACE_WEIGHT_0 + i * 4, data);
	}
	for (i = 0; i < ARRAY_SIZE(nr2d->color_curve); i++) {
		u32 data = (nr2d->color_curve[i][0] << (isp->bits - 8)) | (nr2d->color_curve[i][1] << 16);
		isp_write(isp, ISP_REG_2DNR_COLOR_CURVE_0 + i * 4, data);
	}
	return 0;
}

static int isp_get_ee(struct isp_state *isp, struct xil_isp_lite_ee *ee)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	ee->enabled = !!(top_en & ISP_REG_TOP_EN_BIT_EE_EN);
	return 0;
}

static int isp_set_ee(struct isp_state *isp, const struct xil_isp_lite_ee *ee)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = ee->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_EE_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_EE_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	return 0;
}

static int isp_get_stat_ae_cfg(struct isp_state *isp, struct xil_isp_lite_stat_ae_cfg *ae_cfg)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	ae_cfg->enabled = !!(top_en & ISP_REG_TOP_EN_BIT_STAT_AE_EN);
	ae_cfg->rect_x = isp_read(isp, ISP_REG_STAT_AE_RECT_X);
	ae_cfg->rect_y = isp_read(isp, ISP_REG_STAT_AE_RECT_Y);
	ae_cfg->rect_w = isp_read(isp, ISP_REG_STAT_AE_RECT_W);
	ae_cfg->rect_h = isp_read(isp, ISP_REG_STAT_AE_RECT_H);
	return 0;
}

static int isp_set_stat_ae_cfg(struct isp_state *isp, const struct xil_isp_lite_stat_ae_cfg *ae_cfg)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = ae_cfg->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_STAT_AE_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_STAT_AE_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	isp_write(isp, ISP_REG_STAT_AE_RECT_X, ae_cfg->rect_x);
	isp_write(isp, ISP_REG_STAT_AE_RECT_Y, ae_cfg->rect_y);
	isp_write(isp, ISP_REG_STAT_AE_RECT_W, ae_cfg->rect_w);
	isp_write(isp, ISP_REG_STAT_AE_RECT_H, ae_cfg->rect_h);
	return 0;
}

static int isp_get_stat_awb_cfg(struct isp_state *isp, struct xil_isp_lite_stat_awb_cfg *awb_cfg)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	awb_cfg->enabled = !!(top_en & ISP_REG_TOP_EN_BIT_STAT_AWB_EN);
	awb_cfg->thresh_min = isp_read(isp, ISP_REG_STAT_AWB_MIN);
	awb_cfg->thresh_max = isp_read(isp, ISP_REG_STAT_AWB_MAX);
	return 0;
}

static int isp_set_stat_awb_cfg(struct isp_state *isp, const struct xil_isp_lite_stat_awb_cfg *awb_cfg)
{
	u32 top_en = isp_read(isp, ISP_REG_TOP_EN);
	top_en = awb_cfg->enabled ?
		(top_en |  ISP_REG_TOP_EN_BIT_STAT_AWB_EN) :
		(top_en & ~ISP_REG_TOP_EN_BIT_STAT_AWB_EN) ;
	isp_write(isp, ISP_REG_TOP_EN, top_en);
	isp_write(isp, ISP_REG_STAT_AWB_MIN, awb_cfg->thresh_min);
	isp_write(isp, ISP_REG_STAT_AWB_MAX, awb_cfg->thresh_max);
	return 0;
}

static int isp_get_stat_ae_result(struct isp_state *isp, struct xil_isp_lite_stat_ae_result *ae_result)
{
	u32 i;
	ae_result->pix_cnt  = isp_read(isp, ISP_REG_STAT_AE_PIX_CNT_L);
	ae_result->pix_cnt |= (u64)isp_read(isp, ISP_REG_STAT_AE_PIX_CNT_H) << 32;
	ae_result->sum  = isp_read(isp, ISP_REG_STAT_AE_SUM_L);
	ae_result->sum |= (u64)isp_read(isp, ISP_REG_STAT_AE_SUM_H) << 32;
	for (i = 0; i < XIL_ISP_LITE_AE_HIST_BIN_NUM; i++) {
		u32 offset = XIL_ISP_LITE_AE_HIST_BIN_NUM * 4 * 0 + i * 4;
		ae_result->hist_r[i] = isp_read(isp, ISP_REG_STAT_AE_HIST_ADDR + offset);
	}
	for (i = 0; i < XIL_ISP_LITE_AE_HIST_BIN_NUM; i++) {
		u32 offset = XIL_ISP_LITE_AE_HIST_BIN_NUM * 4 * 1 + i * 4;
		ae_result->hist_gr[i] = isp_read(isp, ISP_REG_STAT_AE_HIST_ADDR + offset);
	}
	for (i = 0; i < XIL_ISP_LITE_AE_HIST_BIN_NUM; i++) {
		u32 offset = XIL_ISP_LITE_AE_HIST_BIN_NUM * 4 * 2 + i * 4;
		ae_result->hist_gb[i] = isp_read(isp, ISP_REG_STAT_AE_HIST_ADDR + offset);
	}
	for (i = 0; i < XIL_ISP_LITE_AE_HIST_BIN_NUM; i++) {
		u32 offset = XIL_ISP_LITE_AE_HIST_BIN_NUM * 4 * 3 + i * 4;
		ae_result->hist_b[i] = isp_read(isp, ISP_REG_STAT_AE_HIST_ADDR + offset);
	}
	return 0;
}

static int isp_get_stat_awb_result(struct isp_state *isp, struct xil_isp_lite_stat_awb_result *awb_result)
{
	u32 i;
	awb_result->pix_cnt  = isp_read(isp, ISP_REG_STAT_AWB_PIX_CNT_L);
	awb_result->pix_cnt |= (u64)isp_read(isp, ISP_REG_STAT_AWB_PIX_CNT_H) << 32;
	awb_result->sum_r  = isp_read(isp, ISP_REG_STAT_AWB_SUM_R_L);
	awb_result->sum_r |= (u64)isp_read(isp, ISP_REG_STAT_AWB_SUM_R_H) << 32;
	awb_result->sum_g  = isp_read(isp, ISP_REG_STAT_AWB_SUM_G_L);
	awb_result->sum_g |= (u64)isp_read(isp, ISP_REG_STAT_AWB_SUM_G_H) << 32;
	awb_result->sum_b  = isp_read(isp, ISP_REG_STAT_AWB_SUM_B_L);
	awb_result->sum_b |= (u64)isp_read(isp, ISP_REG_STAT_AWB_SUM_B_H) << 32;
	for (i = 0; i < XIL_ISP_LITE_AWB_HIST_BIN_NUM; i++) {
		u32 offset = XIL_ISP_LITE_AWB_HIST_BIN_NUM * 4 * 0 + i * 4;
		awb_result->hist_r[i] = isp_read(isp, ISP_REG_STAT_AWB_HIST_ADDR + offset);
	}
	for (i = 0; i < XIL_ISP_LITE_AWB_HIST_BIN_NUM; i++) {
		u32 offset = XIL_ISP_LITE_AWB_HIST_BIN_NUM * 4 * 1 + i * 4;
		awb_result->hist_g[i] = isp_read(isp, ISP_REG_STAT_AWB_HIST_ADDR + offset);
	}
	for (i = 0; i < XIL_ISP_LITE_AWB_HIST_BIN_NUM; i++) {
		u32 offset = XIL_ISP_LITE_AWB_HIST_BIN_NUM * 4 * 2 + i * 4;
		awb_result->hist_b[i] = isp_read(isp, ISP_REG_STAT_AWB_HIST_ADDR + offset);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// ISP config ctrls

static int isp_config_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct isp_state *isp = container_of(ctrl->handler, struct isp_state, config_ctrls);
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_USER_XIL_ISP_LITE_TOP:
		ret = isp_get_top(isp, (struct xil_isp_lite_top*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_DPC:
		ret = isp_get_dpc(isp, (struct xil_isp_lite_dpc*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_BLC:
		ret = isp_get_blc(isp, (struct xil_isp_lite_blc*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_BNR:
		ret = isp_get_bnr(isp, (struct xil_isp_lite_bnr*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_DGAIN:
		ret = isp_get_dgain(isp, (struct xil_isp_lite_dgain*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_DEMOSAIC:
		ret = isp_get_demosaic(isp, (struct xil_isp_lite_demosaic*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_WB:
		ret = isp_get_wb(isp, (struct xil_isp_lite_wb*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_CCM:
		ret = isp_get_ccm(isp, (struct xil_isp_lite_ccm*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_CSC:
		ret = isp_get_csc(isp, (struct xil_isp_lite_csc*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_GAMMA:
		ret = isp_get_gamma(isp, (struct xil_isp_lite_gamma*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_2DNR:
		ret = isp_get_2dnr(isp, (struct xil_isp_lite_2dnr*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_EE:
		ret = isp_get_ee(isp, (struct xil_isp_lite_ee*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_STAT_AE_CFG:
		ret = isp_get_stat_ae_cfg(isp, (struct xil_isp_lite_stat_ae_cfg*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_STAT_AWB_CFG:
		ret = isp_get_stat_awb_cfg(isp, (struct xil_isp_lite_stat_awb_cfg*)ctrl->p_new.p_u8);
		break;
	default:
		dev_err(isp->dev, "Unrecognised control\n");
		ret = -EINVAL;
	}

	if (ret) {
		dev_err(isp->dev, "%s: Failed getting ctrl \"%s\" (%08x), err %d\n",
			 __func__, ctrl->name, ctrl->id, ret);
		ret = -EIO;
	}

	return ret;
}

static int isp_config_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct isp_state *isp = container_of(ctrl->handler, struct isp_state, config_ctrls);
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_USER_XIL_ISP_LITE_TOP:
		ret = isp_set_top(isp, (struct xil_isp_lite_top*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_DPC:
		ret = isp_set_dpc(isp, (struct xil_isp_lite_dpc*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_BLC:
		ret = isp_set_blc(isp, (struct xil_isp_lite_blc*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_BNR:
		ret = isp_set_bnr(isp, (struct xil_isp_lite_bnr*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_DGAIN:
		ret = isp_set_dgain(isp, (struct xil_isp_lite_dgain*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_DEMOSAIC:
		ret = isp_set_demosaic(isp, (struct xil_isp_lite_demosaic*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_WB:
		ret = isp_set_wb(isp, (struct xil_isp_lite_wb*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_CCM:
		ret = isp_set_ccm(isp, (struct xil_isp_lite_ccm*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_CSC:
		ret = isp_set_csc(isp, (struct xil_isp_lite_csc*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_GAMMA:
		ret = isp_set_gamma(isp, (struct xil_isp_lite_gamma*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_2DNR:
		ret = isp_set_2dnr(isp, (struct xil_isp_lite_2dnr*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_EE:
		ret = isp_set_ee(isp, (struct xil_isp_lite_ee*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_STAT_AE_CFG:
		ret = isp_set_stat_ae_cfg(isp, (struct xil_isp_lite_stat_ae_cfg*)ctrl->p_new.p_u8);
		break;
	case V4L2_CID_USER_XIL_ISP_LITE_STAT_AWB_CFG:
		ret = isp_set_stat_awb_cfg(isp, (struct xil_isp_lite_stat_awb_cfg*)ctrl->p_new.p_u8);
		break;
	default:
		dev_err(isp->dev, "Unrecognised control\n");
		ret = -EINVAL;
	}

	if (ret) {
		dev_err(isp->dev, "%s: Failed setting ctrl \"%s\" (%08x), err %d\n",
			 __func__, ctrl->name, ctrl->id, ret);
		ret = -EIO;
	}

	return ret;
}

static const struct v4l2_ctrl_ops isp_config_ctrl_ops = {
	.g_volatile_ctrl = isp_config_g_volatile_ctrl,
	.s_ctrl = isp_config_s_ctrl,
};

struct isp_config_custom_ctrl {
	const char *name;
	u32 id;
	u32 size;
	u32 flags;
};

static const struct isp_config_custom_ctrl custom_ctrls[] = {
	{
		.name	= "ISP Top control",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_TOP,
		.size	= sizeof(struct xil_isp_lite_top),
		.flags  = 0
	}, {
		.name	= "DPC - Defective Pixel Correction",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_DPC,
		.size	= sizeof(struct xil_isp_lite_dpc),
		.flags  = 0
	}, {
		.name	= "BLC - Black Level Correction",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_BLC,
		.size	= sizeof(struct xil_isp_lite_blc),
		.flags  = 0
	}, {
		.name	= "BNR - Bayer Noise Reduction",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_BNR,
		.size	= sizeof(struct xil_isp_lite_bnr),
		.flags  = 0
	}, {
		.name	= "DGAIN - Digital Gain",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_DGAIN,
		.size	= sizeof(struct xil_isp_lite_dgain),
		.flags  = 0
	}, {
		.name	= "Demosaic - Bayer to RGB",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_DEMOSAIC,
		.size	= sizeof(struct xil_isp_lite_demosaic),
		.flags  = 0
	}, {
		.name	= "WB - White Balance",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_WB,
		.size	= sizeof(struct xil_isp_lite_wb),
		.flags  = 0
	}, {
		.name	= "CCM - Color Correction Matrix",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_CCM,
		.size	= sizeof(struct xil_isp_lite_ccm),
		.flags  = 0
	}, {
		.name	= "CSC - Color Space Conversion (RGB to YUV)",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_CSC,
		.size	= sizeof(struct xil_isp_lite_csc),
		.flags  = 0
	}, {
		.name	= "Gamma - Gamma",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_GAMMA,
		.size	= sizeof(struct xil_isp_lite_gamma),
		.flags  = 0
	}, {
		.name	= "2DNR - Bayer Noise Reduction",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_2DNR,
		.size	= sizeof(struct xil_isp_lite_2dnr),
		.flags  = 0
	}, {
		.name	= "EE - Edge Enhancement",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_EE,
		.size	= sizeof(struct xil_isp_lite_ee),
		.flags  = 0
	}, {
		.name	= "AE_CFG - Configure to Auto Exposure Statistics",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_STAT_AE_CFG,
		.size	= sizeof(struct xil_isp_lite_stat_ae_cfg),
		.flags  = 0
	}, {
		.name	= "AWB_CFG - Configure to Auto White Balance Statistics",
		.id	= V4L2_CID_USER_XIL_ISP_LITE_STAT_AWB_CFG,
		.size	= sizeof(struct xil_isp_lite_stat_awb_cfg),
		.flags  = 0
	}
};

static int isp_config_init_ctrl_handler(struct isp_state *isp)
{
	struct v4l2_ctrl_handler *ctrl_handler = &isp->config_ctrls;
	int ret;
	unsigned int i;

	/* Use this ctrl template to assign custom ISP ctrls. */
	struct v4l2_ctrl_config ctrl_template = {
		.ops		= &isp_config_ctrl_ops,
		.type		= V4L2_CTRL_TYPE_U8,
		.def		= 0,
		.min		= 0x00,
		.max		= 0xff,
		.step		= 1,
	};

	/* 3 standard controls, and an array of custom controls */
	ret = v4l2_ctrl_handler_init(ctrl_handler, ARRAY_SIZE(custom_ctrls));
	if (ret) {
		dev_err(isp->dev, "ctrl_handler init failed (%d)", ret);
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(custom_ctrls); i++) {
		ctrl_template.name = custom_ctrls[i].name;
		ctrl_template.id = custom_ctrls[i].id;
		ctrl_template.dims[0] = custom_ctrls[i].size;
		ctrl_template.flags = custom_ctrls[i].flags;
		v4l2_ctrl_new_custom(ctrl_handler, &ctrl_template, NULL);
	}

	if (ctrl_handler->error) {
		ret = ctrl_handler->error;
		dev_err(isp->dev, "controls init failed (%d)", ret);
		v4l2_ctrl_handler_free(ctrl_handler);
		return ret;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// ISP statistics meta node

#define ISP_STAT_REQ_BUFS_MIN 2
#define ISP_STAT_REQ_BUFS_MAX 8

static int isp_stat_enum_fmt_meta_cap(struct file *file, void *priv,
					  struct v4l2_fmtdesc *f)
{
	struct video_device *video = video_devdata(file);
	//struct isp_stat_node *node = video_get_drvdata(video);

	if (f->index > 0 || f->type != video->queue->type)
		return -EINVAL;

	f->pixelformat = V4L2_META_FMT_XIL_ISP_LITE_STAT;
	return 0;
}

static int isp_stat_g_fmt_meta_cap(struct file *file, void *priv,
				       struct v4l2_format *f)
{
	struct video_device *video = video_devdata(file);
	//struct isp_stat_node *node = video_get_drvdata(video);
	struct v4l2_meta_format *meta = &f->fmt.meta;

	if (f->type != video->queue->type)
		return -EINVAL;

	memset(meta, 0, sizeof(*meta));
	meta->dataformat = V4L2_META_FMT_XIL_ISP_LITE_STAT;
	meta->buffersize = sizeof(struct xil_isp_lite_stat_result);

	return 0;
}

static int isp_stat_querycap(struct file *file,
				 void *priv, struct v4l2_capability *cap)
{
	struct video_device *vdev = video_devdata(file);

	strscpy(cap->driver, ISP_DRIVER_NAME, sizeof(cap->driver));
	strscpy(cap->card, vdev->name, sizeof(cap->card));
	strscpy(cap->bus_info, ISP_BUS_NAME, sizeof(cap->bus_info));

	return 0;
}

/* ISP video device IOCTLs */
static const struct v4l2_ioctl_ops isp_stat_ioctl = {
	.vidioc_reqbufs = vb2_ioctl_reqbufs,
	.vidioc_querybuf = vb2_ioctl_querybuf,
	.vidioc_create_bufs = vb2_ioctl_create_bufs,
	.vidioc_qbuf = vb2_ioctl_qbuf,
	.vidioc_dqbuf = vb2_ioctl_dqbuf,
	.vidioc_prepare_buf = vb2_ioctl_prepare_buf,
	.vidioc_expbuf = vb2_ioctl_expbuf,
	.vidioc_streamon = vb2_ioctl_streamon,
	.vidioc_streamoff = vb2_ioctl_streamoff,
	.vidioc_enum_fmt_meta_cap = isp_stat_enum_fmt_meta_cap,
	.vidioc_g_fmt_meta_cap = isp_stat_g_fmt_meta_cap,
	.vidioc_s_fmt_meta_cap = isp_stat_g_fmt_meta_cap,
	.vidioc_try_fmt_meta_cap = isp_stat_g_fmt_meta_cap,
	.vidioc_querycap = isp_stat_querycap,
	.vidioc_subscribe_event = v4l2_ctrl_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
};

static const struct v4l2_file_operations isp_stat_fops = {
	.mmap = vb2_fop_mmap,
	.unlocked_ioctl = video_ioctl2,
	.poll = vb2_fop_poll,
	.open = v4l2_fh_open,
	.release = vb2_fop_release
};

static int isp_stat_vb2_queue_setup(struct vb2_queue *vq,
					unsigned int *num_buffers,
					unsigned int *num_planes,
					unsigned int sizes[],
					struct device *alloc_devs[])
{
	*num_planes = 1;

	*num_buffers = clamp_t(u32, *num_buffers, ISP_STAT_REQ_BUFS_MIN, ISP_STAT_REQ_BUFS_MAX);

	sizes[0] = sizeof(struct xil_isp_lite_stat_result);

	return 0;
}

static void isp_stat_vb2_buf_queue(struct vb2_buffer *vb)
{
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct isp_stat_buffer *buffer =
		container_of(vbuf, struct isp_stat_buffer, vb);
	struct vb2_queue *vq = vb->vb2_queue;
	struct isp_stat_node *node = vq->drv_priv;


	spin_lock_irq(&node->lock);
	list_add_tail(&buffer->list_node, &node->buf_list);
	spin_unlock_irq(&node->lock);
}

static int isp_stat_vb2_buf_prepare(struct vb2_buffer *vb)
{
	if (vb2_plane_size(vb, 0) < sizeof(struct xil_isp_lite_stat_result))
		return -EINVAL;

	vb2_set_plane_payload(vb, 0, sizeof(struct xil_isp_lite_stat_result));

	return 0;
}

static void isp_stat_vb2_stop_streaming(struct vb2_queue *vq)
{
	struct isp_stat_node *node = vq->drv_priv;
	struct isp_stat_buffer *buffer;
	unsigned int i;

	spin_lock_irq(&node->lock);
	for (i = 0; i < ISP_STAT_REQ_BUFS_MAX; i++) {
		if (list_empty(&node->buf_list))
			break;
		buffer = list_first_entry(&node->buf_list, struct isp_stat_buffer, list_node);
		list_del(&buffer->list_node);
		vb2_buffer_done(&buffer->vb.vb2_buf, VB2_BUF_STATE_ERROR);
	}
	spin_unlock_irq(&node->lock);
}

static const struct vb2_ops isp_stat_vb2_ops = {
	.queue_setup = isp_stat_vb2_queue_setup,
	.buf_queue = isp_stat_vb2_buf_queue,
	.buf_prepare = isp_stat_vb2_buf_prepare,
	.wait_prepare = vb2_ops_wait_prepare,
	.wait_finish = vb2_ops_wait_finish,
	.stop_streaming = isp_stat_vb2_stop_streaming,
};

static int isp_stat_init_vb2_queue(struct isp_stat_node *node)
{
	struct vb2_queue *q = &node->queue;
	q->type = V4L2_BUF_TYPE_META_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	q->drv_priv = node;
	q->ops = &isp_stat_vb2_ops;
	q->mem_ops = &vb2_vmalloc_memops;
	q->buf_struct_size = sizeof(struct isp_stat_buffer);
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	q->lock = &node->vlock;

	return vb2_queue_init(q);
}

static void isp_stat_send_measurement(struct isp_stat_node *node)
{
	struct isp_state *isp = container_of(node, struct isp_state, stat_node);
	struct isp_stat_buffer *buffer = NULL;
	unsigned int frame_sequence = isp->frame_sequence;
	u64 timestamp = ktime_get_ns();

	spin_lock(&node->lock);

	/* get one empty buffer */
	if (!list_empty(&node->buf_list)) {
		buffer = list_first_entry(&node->buf_list, struct isp_stat_buffer, list_node);
		list_del(&buffer->list_node);
	}

	if (buffer) {
		struct xil_isp_lite_stat_result *stat_result = NULL;
		stat_result = (struct xil_isp_lite_stat_result *)vb2_plane_vaddr(&buffer->vb.vb2_buf, 0);

		isp_get_stat_ae_result(isp, &stat_result->ae);
		isp_get_stat_awb_result(isp, &stat_result->awb);

		vb2_set_plane_payload(&buffer->vb.vb2_buf, 0, sizeof(struct xil_isp_lite_stat_result));
		buffer->vb.sequence = frame_sequence;
		buffer->vb.vb2_buf.timestamp = timestamp;
		vb2_buffer_done(&buffer->vb.vb2_buf, VB2_BUF_STATE_DONE);
	}

	spin_unlock(&node->lock);
}

static int isp_stat_node_register(struct isp_state *isp, struct v4l2_device *v4l2_dev)
{
	struct isp_stat_node *node = &isp->stat_node;
	struct video_device *vdev = &node->vdev;
	int ret;

	mutex_init(&node->vlock);
	INIT_LIST_HEAD(&node->buf_list);
	spin_lock_init(&node->lock);

	//XXX
	strscpy(vdev->name, ISP_STAT_DEV_NAME, sizeof(vdev->name));

	video_set_drvdata(vdev, node);
	vdev->ioctl_ops = &isp_stat_ioctl;
	vdev->fops = &isp_stat_fops;
	vdev->release = video_device_release_empty;
	vdev->lock = &node->vlock;
	vdev->v4l2_dev = v4l2_dev; //XXX MUST BE SET
	vdev->ctrl_handler = &isp->config_ctrls; //XXX
	vdev->queue = &node->queue;
	vdev->device_caps = V4L2_CAP_META_CAPTURE | V4L2_CAP_STREAMING;
	vdev->vfl_dir =  VFL_DIR_RX;
	isp_stat_init_vb2_queue(node);
	video_set_drvdata(vdev, node);

	node->pad.flags = MEDIA_PAD_FL_SINK;
	ret = media_entity_pads_init(&vdev->entity, 1, &node->pad);
	if (ret)
		goto err_mutex_destroy;

	ret = video_register_device(vdev, VFL_TYPE_VIDEO, -1);
	if (ret) {
		dev_err(&vdev->dev,
			"failed to register %s, ret=%d\n", vdev->name, ret);
		goto err_cleanup_media_entity;
	}

	return 0;

err_cleanup_media_entity:
	media_entity_cleanup(&vdev->entity);
err_mutex_destroy:
	mutex_destroy(&node->vlock);
	return ret;
}

static void isp_stat_node_unregister(struct isp_state *isp)
{
	struct isp_stat_node *node = &isp->stat_node;
	struct video_device *vdev = &node->vdev;

	vb2_video_unregister_device(vdev);
	media_entity_cleanup(&vdev->entity);
	mutex_destroy(&node->vlock);
}

/////////////////////////////////////////////////////////////////////////////
// isp subdev

static int isp_log_status(struct v4l2_subdev *sd)
{
	struct isp_state *isp = to_ispstate(sd);
	struct device *dev = isp->dev;
	u32 width, height, bits, bayer;

	mutex_lock(&isp->lock);

	width  = isp_read(isp, ISP_REG_WIDTH);
	height = isp_read(isp, ISP_REG_HEIGHT);
	bits   = isp_read(isp, ISP_REG_BITS);
	bayer  = isp_read(isp, ISP_REG_BAYER);
	dev_info(dev, "ISP Lite %u x %u RAW%u Bayer %u, stat_result_size %lu",
			width, height, bits, bayer, sizeof(struct xil_isp_lite_stat_result));

	mutex_unlock(&isp->lock);

	return 0;
}

static int isp_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			       struct v4l2_event_subscription *sub)
{
	if (sub->type != V4L2_EVENT_FRAME_SYNC)
		return -EINVAL;

	/* V4L2_EVENT_FRAME_SYNC doesn't require an id, so zero should be set */
	if (sub->id != 0)
		return -EINVAL;

	return v4l2_event_subscribe(fh, sub, 0, NULL);
}

static int isp_start_stream(struct isp_state *isp)
{
	isp_write(isp, ISP_REG_INT_STATUS, 0);
	//isp_write(isp, ISP_REG_INT_MASK, ~(ISP_REG_INT_MASK_BIT_FRAME_START|ISP_REG_INT_MASK_BIT_FRAME_DONE|ISP_REG_INT_MASK_BIT_AE_DONE|ISP_REG_INT_MASK_BIT_AWB_DONE));
	isp_write(isp, ISP_REG_INT_MASK, ~(ISP_REG_INT_MASK_BIT_FRAME_START|ISP_REG_INT_MASK_BIT_FRAME_DONE));
	isp_write(isp, ISP_REG_RESET, 0);

	isp->frame_sequence = 0;
	isp->int_status = 0;
	isp->streaming = true;

	return 0;
}

static void isp_stop_stream(struct isp_state *isp)
{
	isp_write(isp, ISP_REG_RESET, 1);
	isp_write(isp, ISP_REG_INT_MASK, ~0U);
	isp_write(isp, ISP_REG_INT_STATUS, 0);

	isp->streaming = false;
}

static void isp_queue_event_frame_sync(struct isp_state *isp, u32 frame_seq)
{
	struct v4l2_event event = {
		.type = V4L2_EVENT_FRAME_SYNC,
	};
	event.u.frame_sync.frame_sequence = frame_seq;

	v4l2_event_queue(isp->subdev.devnode, &event);
}

static irqreturn_t isp_irq_handler(int irq, void *data)
{
	struct isp_state *isp = (struct isp_state *)data;
	//struct device *dev = isp->dev;
	u32 status, done_mask;

	status = isp_read(isp, ISP_REG_INT_STATUS);
	isp_write(isp, ISP_REG_INT_STATUS, 0);

	//dev_info(dev, "IRQ status %08X", status);

	if (status & ISP_REG_INT_STATUS_BIT_FRAME_START) {
		//dev_info(dev, "IRQ FRAME_START");
		isp_queue_event_frame_sync(isp, isp->frame_sequence);
		isp->int_status = ISP_REG_INT_STATUS_BIT_FRAME_START;
		return IRQ_HANDLED;
	}

#if 0
	if (status & ISP_REG_INT_STATUS_BIT_AE_DONE) {
		dev_info(dev, "IRQ AE_DONE");
		static struct xil_isp_lite_stat_ae_result ae_result;
		u64 hist_r_cnt = 0;
		u64 hist_gr_cnt = 0;
		u64 hist_gb_cnt = 0;
		u64 hist_b_cnt = 0;
		u32 i;
		isp_get_stat_ae_result(isp, &ae_result);
		for (i = 0; i < XIL_ISP_LITE_AE_HIST_BIN_NUM; i ++) {
			hist_r_cnt += ae_result.hist_r[i];
			hist_gr_cnt += ae_result.hist_gr[i];
			hist_gb_cnt += ae_result.hist_gb[i];
			hist_b_cnt += ae_result.hist_b[i];
		}
		dev_info(dev, "IRQ AE_DONE avg-raw %llu, pix_cnt %llu, sum %llu, r_cnt %llu, gr_cnt %llu, gb_cnt %llu, b_cnt %llu",
					ae_result.sum/ae_result.pix_cnt, ae_result.pix_cnt, ae_result.sum,
					hist_r_cnt, hist_gr_cnt, hist_gb_cnt, hist_b_cnt);
	}
	if (status & ISP_REG_INT_STATUS_BIT_AWB_DONE) {
		dev_info(dev, "IRQ AWB_DONE");
		static struct xil_isp_lite_stat_awb_result awb_result;
		u64 hist_r_cnt = 0;
		u64 hist_g_cnt = 0;
		u64 hist_b_cnt = 0;
		u32 i;
		isp_get_stat_awb_result(isp, &awb_result);
		for (i = 0; i < XIL_ISP_LITE_AWB_HIST_BIN_NUM; i ++) {
			hist_r_cnt += awb_result.hist_r[i];
			hist_g_cnt += awb_result.hist_g[i];
			hist_b_cnt += awb_result.hist_b[i];
		}
		dev_info(dev, "IRQ AWB_DONE avg-rgb %llu,%llu,%llu, pix_cnt %llu, sum_r %llu, sum_g %llu, sum_b %llu, r_cnt %llu, g_cnt %llu, b_cnt %llu",
					awb_result.sum_r/awb_result.pix_cnt,
					awb_result.sum_g/awb_result.pix_cnt,
					awb_result.sum_b/awb_result.pix_cnt,
					awb_result.pix_cnt,
					awb_result.sum_r, awb_result.sum_g, awb_result.sum_b,
					hist_r_cnt, hist_g_cnt, hist_b_cnt);
	}
	if (status & ISP_REG_INT_STATUS_BIT_FRAME_DONE) {
		dev_info(dev, "IRQ FRAME_DONE");
	}
#endif

	isp->int_status |= status;
	done_mask  = ISP_REG_INT_STATUS_BIT_FRAME_START | ISP_REG_INT_STATUS_BIT_FRAME_DONE;
	//done_mask |= ISP_REG_INT_STATUS_BIT_AE_DONE | ISP_REG_INT_STATUS_BIT_AWB_DONE;

	if ((isp->int_status & done_mask) == done_mask) {
		//dev_info(dev, "Statistics done");
		isp_stat_send_measurement(&isp->stat_node);
		isp->frame_sequence ++;
		isp->int_status = 0;
	}

	return IRQ_HANDLED;
}

static int isp_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct isp_state *isp = to_ispstate(sd);
	int ret = 0;

	mutex_lock(&isp->lock);

	if (enable == isp->streaming) {
		goto stream_done;
	}

	if (enable) {
		ret = isp_start_stream(isp);
	} else {
		isp_stop_stream(isp);
	}

stream_done:
	mutex_unlock(&isp->lock);
	return ret;
}

static struct v4l2_mbus_framefmt *__isp_get_pad_format(struct isp_state *isp,
			   struct v4l2_subdev_state *sd_state,
			   unsigned int pad, u32 which)
{
	struct v4l2_mbus_framefmt *get_fmt;

	switch (which) {
	case V4L2_SUBDEV_FORMAT_TRY:
		get_fmt = v4l2_subdev_get_try_format(&isp->subdev,
						     sd_state, pad);
		break;
	case V4L2_SUBDEV_FORMAT_ACTIVE:
		get_fmt = &isp->pad_format[pad];
		break;
	default:
		get_fmt = NULL;
		break;
	}

	return get_fmt;
}

static int isp_init_cfg(struct v4l2_subdev *sd,
			      struct v4l2_subdev_state *sd_state)
{
	struct isp_state *isp = to_ispstate(sd);
	struct v4l2_mbus_framefmt *format;
	unsigned int i;

	mutex_lock(&isp->lock);
	for (i = 0; i < ISP_MEDIA_PADS; i++) {
		format = v4l2_subdev_get_try_format(sd, sd_state, i);
		*format = isp->pad_format[i];
	}
	mutex_unlock(&isp->lock);

	return 0;
}

static int isp_get_format(struct v4l2_subdev *sd,
				struct v4l2_subdev_state *sd_state,
				struct v4l2_subdev_format *fmt)
{
	struct isp_state *isp = to_ispstate(sd);
	struct v4l2_mbus_framefmt *get_fmt;
	int ret = 0;

	mutex_lock(&isp->lock);

	get_fmt = __isp_get_pad_format(isp, sd_state, fmt->pad,
					     fmt->which);
	if (!get_fmt) {
		ret = -EINVAL;
		goto unlock_get_format;
	}

	fmt->format = *get_fmt;

unlock_get_format:
	mutex_unlock(&isp->lock);

	return ret;
}

static int isp_set_format(struct v4l2_subdev *sd,
				struct v4l2_subdev_state *sd_state,
				struct v4l2_subdev_format *fmt)
{
	struct isp_state *isp = to_ispstate(sd);
	struct v4l2_mbus_framefmt *__format;
	int ret = 0;

	mutex_lock(&isp->lock);

	__format = __isp_get_pad_format(isp, sd_state,
					      fmt->pad, fmt->which);
	if (!__format) {
		ret = -EINVAL;
		goto unlock_set_format;
	}

	/* only sink pad format can be updated */
	if (fmt->pad != ISP_PAD_SINK) {
		fmt->format = *__format;
		goto unlock_set_format;
	}

	if ((isp->bits == 8 && fmt->format.code == MEDIA_BUS_FMT_Y8_1X8) ||
	    (isp->bits == 10 && fmt->format.code == MEDIA_BUS_FMT_Y10_1X10) ||
	    (isp->bits == 12 && fmt->format.code == MEDIA_BUS_FMT_Y12_1X12)) {
		__format->code = fmt->format.code;
	}
	fmt->format = *__format;

unlock_set_format:
	mutex_unlock(&isp->lock);

	return ret;
}

static int isp_enum_mbus_code(struct v4l2_subdev *sd,
				    struct v4l2_subdev_state *sd_state,
				    struct v4l2_subdev_mbus_code_enum *code)
{
	struct isp_state *isp = to_ispstate(sd);

	if (code->index > 1 || code->pad >= ISP_MEDIA_PADS) {
		return -EINVAL;
	}

	if (code->index == 0) {
		code->code = isp->pad_format[code->pad].code;
	} else {
		code->code = isp->bits == 8 ? MEDIA_BUS_FMT_Y8_1X8 : (isp->bits == 10) ? MEDIA_BUS_FMT_Y10_1X10 : MEDIA_BUS_FMT_Y12_1X12;
	}

	return 0;
}

static int isp_enum_frame_size(struct v4l2_subdev *sd,
			       struct v4l2_subdev_state *state,
			       struct v4l2_subdev_frame_size_enum *fse)
{
	struct isp_state *isp = to_ispstate(sd);

	if (fse->index > 0 || fse->pad >= ISP_MEDIA_PADS || fse->code != isp->pad_format[fse->pad].code) {
		return -EINVAL;
	}

	fse->min_width  = isp->pad_format[fse->pad].width;
	fse->max_width  = isp->pad_format[fse->pad].width;
	fse->min_height = isp->pad_format[fse->pad].height;
	fse->max_height = isp->pad_format[fse->pad].height;

	return 0;
}

/* -----------------------------------------------------------------------------
 * Media Operations
 */

static const struct media_entity_operations isp_media_ops = {
	.link_validate = v4l2_subdev_link_validate
};

static const struct v4l2_subdev_core_ops isp_core_ops = {
	.log_status = isp_log_status,
	.subscribe_event = isp_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static const struct v4l2_subdev_video_ops isp_video_ops = {
	.s_stream = isp_s_stream,
};

static const struct v4l2_subdev_pad_ops isp_pad_ops = {
	.init_cfg = isp_init_cfg,
	.enum_mbus_code = isp_enum_mbus_code,
	.enum_frame_size = isp_enum_frame_size,
	.get_fmt = isp_get_format,
	.set_fmt = isp_set_format,
	.link_validate = v4l2_subdev_link_validate_default,
};

static const struct v4l2_subdev_ops isp_ops = {
	.core = &isp_core_ops,
	.video = &isp_video_ops,
	.pad = &isp_pad_ops
};

static int isp_internal_registered (struct v4l2_subdev *sd)
{
	struct isp_state *isp = to_ispstate(sd);
	return isp_stat_node_register(isp, sd->v4l2_dev);
}

static void isp_internal_unregistered (struct v4l2_subdev *sd)
{
	struct isp_state *isp = to_ispstate(sd);
	isp_stat_node_unregister(isp);
}

static const struct v4l2_subdev_internal_ops isp_internal_ops = {
	.registered    = isp_internal_registered,
	.unregistered  = isp_internal_unregistered,
};

static int isp_get_hw_format(struct isp_state *isp)
{
	struct device *dev = isp->dev;
	struct v4l2_mbus_framefmt *format = NULL;

	const u32 raw8_codes[] =  {MEDIA_BUS_FMT_SRGGB8_1X8, MEDIA_BUS_FMT_SGRBG8_1X8, MEDIA_BUS_FMT_SGBRG8_1X8, MEDIA_BUS_FMT_SBGGR8_1X8};
	const u32 raw10_codes[] = {MEDIA_BUS_FMT_SRGGB10_1X10, MEDIA_BUS_FMT_SGRBG10_1X10, MEDIA_BUS_FMT_SGBRG10_1X10, MEDIA_BUS_FMT_SBGGR10_1X10};
	const u32 raw12_codes[] = {MEDIA_BUS_FMT_SRGGB12_1X12, MEDIA_BUS_FMT_SGRBG12_1X12, MEDIA_BUS_FMT_SGBRG12_1X12, MEDIA_BUS_FMT_SBGGR12_1X12};

	u32 width  = isp_read(isp, ISP_REG_WIDTH);
	u32 height = isp_read(isp, ISP_REG_HEIGHT);
	u32 bits   = isp_read(isp, ISP_REG_BITS);
	u32 bayer  = isp_read(isp, ISP_REG_BAYER);
	if (width < 1 || height < 1 || (bits != 8 && bits != 10 && bits != 12) || bayer >= 4) {
		dev_err(dev, "Invalid HW formats. Resolution %u x %u, RAW%u, Bayer %u",
			width, height, bits, bayer);
		return -EINVAL;
	}

	dev_info(dev, "ISP HW formats. Resolution %u x %u, RAW%u, Bayer %u",
		width, height, bits, bayer);
	
	isp->bits = bits;
	format = &isp->pad_format[ISP_PAD_SINK];

	switch (bits) {
		case 8:   format->code = raw8_codes[bayer];  break;
		case 10:  format->code = raw10_codes[bayer]; break;
		default:  format->code = raw12_codes[bayer]; break;
	}

	format->field      = V4L2_FIELD_NONE;
	format->colorspace = V4L2_COLORSPACE_SRGB;
	format->width      = width;
	format->height     = height;

	format = &isp->pad_format[ISP_PAD_SOURCE];
	*format = isp->pad_format[ISP_PAD_SINK];
	switch (bits) {
		case 8:   format->code = MEDIA_BUS_FMT_YUV8_1X24;  break;
		case 10:  format->code = MEDIA_BUS_FMT_YUV10_1X30; break;
		default:  format->code = MEDIA_BUS_FMT_YUV12_1X36; break;
	}
#if ISP_MEDIA_PADS > 2
	format = &isp->pad_format[ISP_PAD_SOURCE_2];
	*format = isp->pad_format[ISP_PAD_SINK];
	switch (bits) {
		case 8:   format->code = MEDIA_BUS_FMT_YUV8_1X24;  break;
		case 10:  format->code = MEDIA_BUS_FMT_YUV10_1X30; break;
		default:  format->code = MEDIA_BUS_FMT_YUV12_1X36; break;
	}
#endif

	return 0;
}

//np.uint8(np.power(np.double(range(64))/63, 1/2.2) * 63)
static const unsigned gamma_table[] = {
        0,  9, 13, 15, 17, 19, 21, 23, 24, 26, 27, 28, 29, 30, 31, 32, 33,
       34, 35, 36, 37, 38, 39, 39, 40, 41, 42, 42, 43, 44, 44, 45, 46, 46,
       47, 48, 48, 49, 50, 50, 51, 51, 52, 52, 53, 54, 54, 55, 55, 56, 56,
       57, 57, 58, 58, 59, 59, 60, 60, 61, 61, 62, 62, 63
};

//spaceKernel = x_bf_makeSpaceKern(7, 6, 31); print(spaceKernel)
static const unsigned char spaceWeightTbl_6[7*7] = {
		24, 26, 27, 27, 27, 26, 24,
		26, 28, 29, 29, 29, 28, 26,
		27, 29, 30, 31, 30, 29, 27,
		27, 29, 31, 31, 31, 29, 27,
		27, 29, 30, 31, 30, 29, 27,
		26, 28, 29, 29, 29, 28, 26,
		24, 26, 27, 27, 27, 26, 24
	};
//spaceKernel = x_bf_makeSpaceKern(7, 8, 31); print(spaceKernel)
static const unsigned char spaceWeightTbl_8[7*7] = {
		27, 28, 29, 29, 29, 28, 27,
		28, 29, 30, 30, 30, 29, 28,
		29, 30, 31, 31, 31, 30, 29,
		29, 30, 31, 31, 31, 30, 29,
		29, 30, 31, 31, 31, 30, 29,
		28, 29, 30, 30, 30, 29, 28,
		27, 28, 29, 29, 29, 28, 27
	};
//spaceKernel = x_bf_makeSpaceKern(7, 10, 31); print(spaceKernel)
static const unsigned char spaceWeightTbl_10[7*7] = {
		28, 29, 29, 30, 29, 29, 28,
		29, 30, 30, 30, 30, 30, 29,
		29, 30, 31, 31, 31, 30, 29,
		30, 30, 31, 31, 31, 30, 30,
		29, 30, 31, 31, 31, 30, 29,
		29, 30, 30, 30, 30, 30, 29,
		28, 29, 29, 30, 29, 29, 28
	};
//colorCurve = x_bf_makeColorCurve(9, 20, 6, 31); print(colorCurve)
static const unsigned char colorCurveTbl_6[9][2] = {
		{ 2, 29},
		{ 4, 25},
		{ 6, 19},
		{ 8, 13},
		{10,  8},
		{12,  4},
		{14,  2},
		{16,  1},
		{18,  0}
	};
//colorCurve = x_bf_makeColorCurve(9, 26, 8, 31); print(colorCurve)
static const unsigned char colorCurveTbl_8[9][2] = {
		{ 2, 30},
		{ 5, 25},
		{ 7, 21},
		{10, 14},
		{13,  8},
		{15,  5},
		{18,  2},
		{20,  1},
		{23,  0}
	};
//colorCurve = x_bf_makeColorCurve(9, 34, 10, 31); print(colorCurve)
static const unsigned char colorCurveTbl_10[9][2] = {
		{ 3, 30},
		{ 6, 26},
		{10, 19},
		{13, 13},
		{17,  7},
		{20,  4},
		{23,  2},
		{27,  1},
		{30,  0}
	};

static int isp_initialize_hw(struct isp_state *isp)
{
	u32 width  = isp->pad_format[ISP_PAD_SINK].width;
	u32 height = isp->pad_format[ISP_PAD_SINK].height;
	u32 bits   = isp->bits;
	//struct xil_isp_lite_top top = {};
	struct xil_isp_lite_dpc dpc = {};
	struct xil_isp_lite_blc blc = {};
	struct xil_isp_lite_bnr bnr = {};
	struct xil_isp_lite_dgain dgain = {};
	struct xil_isp_lite_demosaic demosaic = {};
	struct xil_isp_lite_wb wb = {};
	struct xil_isp_lite_ccm ccm = {};
	struct xil_isp_lite_csc csc = {};
	struct xil_isp_lite_gamma gamma = {};
	struct xil_isp_lite_2dnr nr2d = {};
	struct xil_isp_lite_ee ee = {};
	struct xil_isp_lite_stat_ae_cfg ae_cfg = {};
	struct xil_isp_lite_stat_awb_cfg awb_cfg = {};
	unsigned i;

	isp_write(isp, ISP_REG_RESET, 1);
	isp_write(isp, ISP_REG_INT_MASK, ~0U);

	dpc.enabled = 1;
	blc.enabled = 1;
	bnr.enabled = 1;
	dgain.enabled = 1;
	demosaic.enabled = 1;
	wb.enabled = 1;
	ccm.enabled = 1;
	csc.enabled = 1;
	gamma.enabled = 1;
	nr2d.enabled = 1;
	ee.enabled = 1;
	ae_cfg.enabled = 1;
	awb_cfg.enabled = 1;

	dpc.threshold = 32 << (bits - 8);

	blc.black_level_r  = 10 << (bits - 8);
	blc.black_level_gr = 10 << (bits - 8);
	blc.black_level_gb = 10 << (bits - 8);
	blc.black_level_b  = 10 << (bits - 8);

	bnr.nr_level = 2;

	dgain.gain = 0x10;
	dgain.offset = 0;
	
	wb.ggain = 0x10;
	wb.rgain = 0x17;
	wb.bgain = 0x20;

	ccm.matrix[0] =  0x1a;
	ccm.matrix[1] = -0x05;
	ccm.matrix[2] = -0x05;
	ccm.matrix[3] = -0x05;
	ccm.matrix[4] =  0x1a;
	ccm.matrix[5] = -0x05;
	ccm.matrix[6] = -0x05;
	ccm.matrix[7] = -0x05;
	ccm.matrix[8] =  0x1a;

	for (i = 0; i < ARRAY_SIZE(gamma.gamma_table); i++) {
		gamma.gamma_table[i] = gamma_table[i];
	}

	for (i = 0; i < ARRAY_SIZE(nr2d.space_weight); i++) {
		nr2d.space_weight[i] = spaceWeightTbl_10[i];
	}
	for (i = 0; i < ARRAY_SIZE(nr2d.color_curve); i++) {
		nr2d.color_curve[i][0] = colorCurveTbl_10[i][0];
		nr2d.color_curve[i][1] = colorCurveTbl_10[i][1];
	}

	ae_cfg.rect_x = width / 8;
	ae_cfg.rect_y = height / 8;
	ae_cfg.rect_w = width / 4 * 3;
	ae_cfg.rect_h = height / 4 * 3;

	awb_cfg.thresh_min = 10 << (bits - 8);
	awb_cfg.thresh_max = 220 << (bits - 8);

	isp_set_dpc(isp, &dpc);
	isp_set_blc(isp, &blc);
	isp_set_bnr(isp, &bnr);
	isp_set_dgain(isp, &dgain);
	isp_set_demosaic(isp, &demosaic);
	isp_set_wb(isp, &wb);
	isp_set_ccm(isp, &ccm);
	isp_set_csc(isp, &csc);
	isp_set_gamma(isp, &gamma);
	isp_set_2dnr(isp, &nr2d);
	isp_set_ee(isp, &ee);
	isp_set_stat_ae_cfg(isp, &ae_cfg);
	isp_set_stat_awb_cfg(isp, &awb_cfg);

	// isp_get_top(isp, &top);
	// dev_info(isp->dev, "top_en 0x%04X", *(u32*)&top);

	// isp_get_gamma(isp, &gamma);
	// for (i = 0; i < ARRAY_SIZE(gamma.gamma_table); i++) {
	// 	dev_info(isp->dev, "gamma[%u]:\t%u", i, gamma.gamma_table[i]);
	// }

	// isp_get_ccm(isp, &ccm);
	// for (i = 0; i < ARRAY_SIZE(ccm.matrix); i++) {
	// 	dev_info(isp->dev, "matrix[%u]:\t%02X, %d", i, (u8)ccm.matrix[i], ccm.matrix[i]);
	// }
	return 0;
}

static const struct clk_bulk_data isp_clks[] = {
	{ .id = "s00_axi_aclk" },
	{ .id = "pclk" },
};

static int isp_probe(struct platform_device *pdev)
{
	struct v4l2_subdev *subdev;
	struct isp_state *isp;
	int num_clks = ARRAY_SIZE(isp_clks);
	struct device *dev = &pdev->dev;
	int irq, ret;

	isp = devm_kzalloc(dev, sizeof(*isp), GFP_KERNEL);
	if (!isp) {
		dev_err(dev, "No memory for isp");
		return -ENOMEM;
	}

	isp->dev = dev;

	isp->clks = devm_kmemdup(dev, isp_clks, sizeof(isp_clks), GFP_KERNEL);
	if (!isp->clks) {
		dev_err(dev, "No memory for isp clks");
		return -ENOMEM;
	}

	isp->iomem = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(isp->iomem)) {
		dev_err(dev, "No iomem resource in DT");
		return PTR_ERR(isp->iomem);
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "No irq resource in DT");
		return irq;
	}

	ret = devm_request_threaded_irq(dev, irq, NULL,
					isp_irq_handler, IRQF_ONESHOT,
					dev_name(dev), isp);
	if (ret) {
		dev_err(dev, "Err = %d Interrupt handler reg failed!\n", ret);
		return ret;
	}

	ret = devm_clk_bulk_get(dev, num_clks, isp->clks);
	if (ret) {
		dev_err(dev, "could not get clks");
		return ret;
	}

	/* TODO: Enable/disable clocks at stream on/off time. */
	ret = clk_bulk_prepare_enable(num_clks, isp->clks);
	if (ret) {
		dev_err(dev, "could not prepare enable clks");
		return ret;
	}

	mutex_init(&isp->lock);

	/* Initialize V4L2 subdevice and media entity */
	isp->pads[ISP_PAD_SINK].flags = MEDIA_PAD_FL_SINK;
	isp->pads[ISP_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;
#if ISP_MEDIA_PADS > 2
	isp->pads[ISP_PAD_SOURCE_2].flags = MEDIA_PAD_FL_SOURCE;
#endif

	/* Initialize the default format */
	ret = isp_get_hw_format(isp);
	if (ret < 0) {
		goto error;
	}

	/* Initialize the isp hardware */
	ret = isp_initialize_hw(isp);
	if (ret < 0) {
		goto error;
	}

	/* Initialize ctrl handler */
	ret = isp_config_init_ctrl_handler(isp);
	if (ret < 0) {
		goto error;
	}

	/* Initialize V4L2 subdevice and media entity */
	subdev = &isp->subdev;
	v4l2_subdev_init(subdev, &isp_ops);
	subdev->dev = dev;
	subdev->internal_ops = &isp_internal_ops;
	strscpy(subdev->name, dev_name(dev), sizeof(subdev->name));
	subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS | V4L2_SUBDEV_FL_HAS_DEVNODE;
	subdev->entity.ops = &isp_media_ops;
	v4l2_set_subdevdata(subdev, isp);

	ret = media_entity_pads_init(&subdev->entity, ISP_MEDIA_PADS,
				     isp->pads);
	if (ret < 0) {
		dev_err(dev, "init media entity pads fail");
		goto error;
	}

	platform_set_drvdata(pdev, isp);

	ret = v4l2_async_register_subdev(subdev);
	if (ret < 0) {
		dev_err(dev, "failed to register subdev\n");
		goto error;
	}

	dev_info(dev, ISP_DRIVER_NAME " driver probed!");

	return 0;
error:
	media_entity_cleanup(&subdev->entity);
	mutex_destroy(&isp->lock);
	clk_bulk_disable_unprepare(num_clks, isp->clks);
	return ret;
}

static int isp_remove(struct platform_device *pdev)
{
	struct isp_state *isp = platform_get_drvdata(pdev);
	struct v4l2_subdev *subdev = &isp->subdev;
	int num_clks = ARRAY_SIZE(isp_clks);

	v4l2_async_unregister_subdev(subdev);
	media_entity_cleanup(&subdev->entity);
	mutex_destroy(&isp->lock);
	clk_bulk_disable_unprepare(num_clks, isp->clks);

	return 0;
}

static const struct of_device_id isp_of_id_table[] = {
	{ .compatible = "xlnx,xil-isp-lite-1.0", },
	{ }
};
MODULE_DEVICE_TABLE(of, isp_of_id_table);

static struct platform_driver isp_driver = {
	.driver = {
		.name		= ISP_DRIVER_NAME,
		.of_match_table	= isp_of_id_table,
	},
	.probe			= isp_probe,
	.remove			= isp_remove,
};

module_platform_driver(isp_driver);

MODULE_AUTHOR("xinquan bian <544177215@qq.com>");
MODULE_DESCRIPTION("Xil ISP Lite Driver");
MODULE_LICENSE("GPL v2");
