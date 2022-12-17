// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx vcu clock consumer driver
 *
 * Copyright (C) 2021 Xilinx, Inc.
 *
 * Author: Rohit Visavalia <rohit.visavalia@xilinx.com>
 *
 * This file is used to set the encoder and decoder clk.
 * it acts as a clk consumer for xilinx VCU.
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/mfd/syscon/xlnx-vcu.h>
#include <linux/regmap.h>
#include <linux/clk.h>

#include "al_codec.h"

#define VCU_PLL_CLK			0x34
#define VCU_PLL_CLK_DEC			0x64
#define VCU_MCU_CLK			0x24
#define VCU_CORE_CLK			0x28
#define MHZ				1000000
#define FRAC				100

int xil_clk_set_up(struct platform_device *pdev, struct al5_codec_desc *codec,
		   const char *core_clk, const char *mcu_clk)
{
	u32 refclk, coreclk, mcuclk, inte, deci;
	int err;
	struct xvcu_device *xvcu_core = dev_get_drvdata(pdev->dev.parent);
	codec->logicore_reg_ba = xvcu_core->logicore_reg_ba;

	codec->pll_ref = devm_clk_get(pdev->dev.parent, "pll_ref");
	if (IS_ERR(codec->pll_ref)) {
		if (PTR_ERR(codec->pll_ref) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Could not get pll_ref clock\n");
		return PTR_ERR(codec->pll_ref);
	}

	codec->core_clk = devm_clk_get(pdev->dev.parent, core_clk);
	if (IS_ERR(codec->core_clk)) {
		if (PTR_ERR(codec->core_clk) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Could not get core clock\n");
		return PTR_ERR(codec->core_clk);
	}

	codec->mcu_clk = devm_clk_get(pdev->dev.parent, mcu_clk);
	if (IS_ERR(codec->mcu_clk)) {
		if (PTR_ERR(codec->mcu_clk) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Could not get mcu clock\n");
		return PTR_ERR(codec->mcu_clk);
	}

	regmap_read(codec->logicore_reg_ba, VCU_PLL_CLK, &inte);
	regmap_read(codec->logicore_reg_ba, VCU_PLL_CLK_DEC, &deci);
	regmap_read(codec->logicore_reg_ba, VCU_CORE_CLK, &coreclk);
	regmap_read(codec->logicore_reg_ba, VCU_MCU_CLK, &mcuclk);

	if (!mcuclk || !coreclk) {
		dev_err(&pdev->dev, "Invalid mcu and core clock data\n");
		return -EINVAL;
	}

	refclk = (inte * MHZ) + (deci * (MHZ / FRAC));
	coreclk *= MHZ;
	mcuclk *= MHZ;
	dev_dbg(&pdev->dev, "Ref clock from logicoreIP is %uHz\n", refclk);
	dev_dbg(&pdev->dev, "Core clock from logicoreIP is %uHz\n", coreclk);
	dev_dbg(&pdev->dev, "Mcu clock from logicoreIP is %uHz\n", mcuclk);

	err = clk_set_rate(codec->pll_ref, refclk);
	if (err)
		dev_warn(&pdev->dev, "failed to set logicoreIP refclk rate %d\n"
			 , err);

	err = clk_prepare_enable(codec->pll_ref);
	if (err) {
		dev_err(&pdev->dev, "failed to enable pll_ref clk source %d\n",
			err);
		return err;
	}

	err = clk_set_rate(codec->mcu_clk, mcuclk);
	if (err)
		dev_warn(&pdev->dev, "failed to set logicoreIP mcu clk rate "
			 "%d\n", err);

	err = clk_prepare_enable(codec->mcu_clk);
	if (err) {
		dev_err(&pdev->dev, "failed to enable mcu %d\n", err);
		goto error_mcu;
	}

	err = clk_set_rate(codec->core_clk, coreclk);
	if (err)
		dev_warn(&pdev->dev, "failed to set logicoreIP core clk rate "
			 "%d\n", err);

	err = clk_prepare_enable(codec->core_clk);
	if (err) {
		dev_err(&pdev->dev, "failed to enable core %d\n", err);
		goto error_core;
	}
	return 0;

error_core:
	clk_disable_unprepare(codec->mcu_clk);
error_mcu:
	clk_disable_unprepare(codec->pll_ref);

	return err;

}
EXPORT_SYMBOL_GPL(xil_clk_set_up);

int xil_clk_clean_up(struct platform_device *pdev, struct al5_codec_desc *codec)
{
	clk_disable_unprepare(codec->core_clk);
	devm_clk_put(pdev->dev.parent, codec->core_clk);

	clk_disable_unprepare(codec->mcu_clk);
	devm_clk_put(pdev->dev.parent, codec->mcu_clk);

	clk_disable_unprepare(codec->pll_ref);
	devm_clk_put(pdev->dev.parent, codec->pll_ref);

	return 0;
}
EXPORT_SYMBOL_GPL(xil_clk_clean_up);

#endif
