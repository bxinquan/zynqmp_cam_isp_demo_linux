/* SPDX-License-Identifier: ((GPL-2.0+ WITH Linux-syscall-note) OR BSD-3-Clause) */
/*
 * xil-isp-lite.h
 *
 * Xil ISP Lite driver - user space header file.
 *
 * Copyright © 2022- bxinquan Ltd.
 *
 * Author: Xinquan Bian (544177215@qq.com)
 *
 */

#ifndef __XIL_ISP_LITE_H_
#define __XIL_ISP_LITE_H_

#include <linux/v4l2-controls.h>

#define V4L2_CID_USER_XIL_ISP_LITE_BASE			(V4L2_CID_USER_BASE + 0x10e0)

#define V4L2_CID_USER_XIL_ISP_LITE_TOP			(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x0001)
#define V4L2_CID_USER_XIL_ISP_LITE_DPC			(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x0002)
#define V4L2_CID_USER_XIL_ISP_LITE_BLC			(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x0003)
#define V4L2_CID_USER_XIL_ISP_LITE_BNR			(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x0004)
#define V4L2_CID_USER_XIL_ISP_LITE_DGAIN		(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x0005)
#define V4L2_CID_USER_XIL_ISP_LITE_DEMOSAIC		(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x0006)
#define V4L2_CID_USER_XIL_ISP_LITE_WB			(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x0007)
#define V4L2_CID_USER_XIL_ISP_LITE_CCM			(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x0008)
#define V4L2_CID_USER_XIL_ISP_LITE_CSC			(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x0009)
#define V4L2_CID_USER_XIL_ISP_LITE_GAMMA		(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x000a)
#define V4L2_CID_USER_XIL_ISP_LITE_2DNR			(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x000b)
#define V4L2_CID_USER_XIL_ISP_LITE_EE			(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x000c)
#define V4L2_CID_USER_XIL_ISP_LITE_STAT_AE_CFG		(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x000d)
#define V4L2_CID_USER_XIL_ISP_LITE_STAT_AWB_CFG		(V4L2_CID_USER_XIL_ISP_LITE_BASE + 0x000e)

/*
 * XIL-ISP-LITE IP Modules
 *
 *        |------------- RAW -------------|   |------- RGB -------| |--------- YUV ---------|
 * RAW ==> DPC => BLC => BNR => DGAIN => Demosaic => WB => CCM => CSC => Gamma => 2DNR => EE ==> YUV
 *                                    |           |
 *                                    V           V
 *                                 STAT_AE    STAT_AWB
 * */


/* ISP Configure structure define */

struct xil_isp_lite_top {
	__u32 dpc_en		:1;
	__u32 blc_en		:1;
	__u32 bnr_en		:1;
	__u32 dgain_en		:1;
	__u32 demosaic_en	:1;
	__u32 wb_en		:1;
	__u32 ccm_en		:1;
	__u32 csc_en		:1;
	__u32 gamma_en		:1;
	__u32 nr2d_en		:1;
	__u32 ee_en		:1;
	__u32 stat_ae_en	:1;
	__u32 stat_awb_en	:1;
	__u32 padding		:19; /* Unused */
};

struct xil_isp_lite_dpc {
	__u32 enabled;
	__u16 threshold;
	__u16 padding;
};

struct xil_isp_lite_blc {
	__u32 enabled;
	__u16 black_level_r;
	__u16 black_level_gr;
	__u16 black_level_gb;
	__u16 black_level_b;
};

struct xil_isp_lite_bnr {
	__u32 enabled;
	__u32 nr_level   :4;
	__u32 padding    :28;
};

struct xil_isp_lite_dgain {
	__u32 enabled;
	__u8  gain;  //U4.4
	__u16 offset;
	__u8  padding;
};

struct xil_isp_lite_demosaic {
	__u32 enabled;
};

struct xil_isp_lite_wb {
	__u32 enabled;
	__u8  rgain; //U4.4
	__u8  ggain;
	__u8  bgain;
	__u8  padding;
};

struct xil_isp_lite_ccm {
	__u32 enabled;
	__s8  matrix[3*3];
	__u8  padding[3];
};

struct xil_isp_lite_csc {
	__u32 enabled;
};

struct xil_isp_lite_gamma {
	__u32 enabled;
	__u8  gamma_table[256];
};

struct xil_isp_lite_2dnr {
	__u32 enabled;
	__u8  space_weight[7*7];
	__u8  color_curve[9][2];
	__u8  padding;
};

struct xil_isp_lite_ee {
	__u32 enabled;
};

struct xil_isp_lite_stat_ae_cfg {
	__u32 enabled;
	__u16 rect_x;
	__u16 rect_y;
	__u16 rect_w;
	__u16 rect_h;
};

struct xil_isp_lite_stat_awb_cfg {
	__u32 enabled;
	__u16 thresh_min;
	__u16 thresh_max;
};


/* ISP statistics buffer define */

#define V4L2_META_FMT_XIL_ISP_LITE_STAT		v4l2_fourcc('X', 'I', 'S', 'P') 
#define XIL_ISP_LITE_AE_HIST_BIN_NUM		(256)
#define XIL_ISP_LITE_AWB_HIST_BIN_NUM		(256)

struct xil_isp_lite_stat_ae_result {
	__u64 pix_cnt;
	__u64 sum;
	__u32 hist_r[XIL_ISP_LITE_AE_HIST_BIN_NUM];
	__u32 hist_gr[XIL_ISP_LITE_AE_HIST_BIN_NUM];
	__u32 hist_gb[XIL_ISP_LITE_AE_HIST_BIN_NUM];
	__u32 hist_b[XIL_ISP_LITE_AE_HIST_BIN_NUM];
};

struct xil_isp_lite_stat_awb_result {
	__u64 pix_cnt;
	__u64 sum_r;
	__u64 sum_g;
	__u64 sum_b;
	__u32 hist_r[XIL_ISP_LITE_AWB_HIST_BIN_NUM];
	__u32 hist_g[XIL_ISP_LITE_AWB_HIST_BIN_NUM];
	__u32 hist_b[XIL_ISP_LITE_AWB_HIST_BIN_NUM];
};

struct xil_isp_lite_stat_result {
	struct xil_isp_lite_stat_ae_result  ae;
	struct xil_isp_lite_stat_awb_result awb;
};

#endif /* __XIL_ISP_LITE_H_ */
