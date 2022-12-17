// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx vcu clock consumer driver header file
 *
 * Copyright (C) 2021 Xilinx, Inc.
 *
 * Author: Rohit Visavalia <rohit.visavalia@xilinx.com>
 *
 * This file is used to set the encoder and decoder clk.
 * it acts as a clk consumer for xilinx VCU.
 */
#ifndef _XIL_CLK_H_
#define _XIL_CLK_H_
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
#include "al_codec.h"

int xil_clk_set_up(struct platform_device *pdev, struct al5_codec_desc *codec,
		   const char *core_clk, const char *mcu_clk);
int xil_clk_clean_up(struct platform_device *pdev,
		     struct al5_codec_desc *codec);

#endif
#endif /* _XIL_CLK_H_ */
