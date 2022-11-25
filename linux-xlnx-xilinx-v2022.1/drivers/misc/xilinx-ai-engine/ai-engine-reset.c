// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx AI Engine device driver resets implementation
 *
 * Copyright (C) 2020 Xilinx, Inc.
 */

#include <linux/bitfield.h>
#include <linux/firmware/xlnx-zynqmp.h>
#include <linux/io.h>

#include "ai-engine-internal.h"

/**
 * aie_part_clear_core_regs_of_tile() - clear registers of aie core
 * @apart: AI engine partition
 * @loc: location of aie tile to clear
 */
static void aie_part_clear_core_regs_of_tile(struct aie_partition *apart,
					     struct aie_location loc)
{
	struct aie_device *adev = apart->adev;
	struct aie_aperture *aperture = apart->aperture;
	const struct aie_core_regs_attr *regs = adev->core_regs;
	u32 i;

	for (i = 0; i < adev->num_core_regs; i++) {
		u32 j, soff, eoff, reg;

		soff = aie_cal_regoff(adev, loc, regs[i].core_regs->soff);
		eoff = aie_cal_regoff(adev, loc, regs[i].core_regs->eoff);

		for (reg = soff; reg <= eoff; reg += AIE_CORE_REGS_STEP) {
			for (j = 0; j < regs[i].width; j++)
				iowrite32(0, aperture->base + reg + j * 4);
		}
	}
}

/**
 * aie_part_clear_core_regs - clear registers of aie core of a partition
 * @apart: AI engine partition
 */
static void aie_part_clear_core_regs(struct aie_partition *apart)
{
	struct aie_range *range = &apart->range;
	u32 c, r;

	/* clear core registers for each tile in the partition */
	for (c = range->start.col; c < range->start.col + range->size.col;
			c++) {
		for (r = range->start.row;
				r < range->start.row + range->size.row; r++) {
			struct aie_location loc;
			u32 ttype;

			loc.row = r;
			loc.col = c;
			ttype = apart->adev->ops->get_tile_type(apart->adev,
								&loc);
			if (ttype == AIE_TILE_TYPE_TILE &&
			    aie_part_check_clk_enable_loc(apart, &loc))
				aie_part_clear_core_regs_of_tile(apart, loc);
		}
	}
}

/**
 * aie_part_clean() - reset and clear AI engine partition
 * @apart: AI engine partition
 * @return: 0 for success and negative value for failure
 *
 * This function will:
 *  * gate all the columns
 *  * reset AI engine partition columns
 *  * reset AI engine shims
 *  * clear the memories
 *  * clear core registers
 *  * gate all the tiles in a partition
 *  * update clock state bitmap
 *
 * This function will not validate the partition, the caller will need to
 * provide a valid AI engine partition.
 */
int aie_part_clean(struct aie_partition *apart)
{
	u32 node_id = apart->adev->pm_node_id;
	int ret;

	if (apart->cntrflag & XAIE_PART_NOT_RST_ON_RELEASE)
		return 0;

	ret = zynqmp_pm_aie_operation(node_id, apart->range.start.col,
				      apart->range.size.col,
				      XILINX_AIE_OPS_DIS_COL_CLK_BUFF);
	if (ret < 0)
		return ret;

	ret = zynqmp_pm_aie_operation(node_id, apart->range.start.col,
				      apart->range.size.col,
				      XILINX_AIE_OPS_COL_RST |
				      XILINX_AIE_OPS_SHIM_RST);
	if (ret < 0)
		return ret;

	ret = zynqmp_pm_aie_operation(node_id, apart->range.start.col,
				      apart->range.size.col,
				      XILINX_AIE_OPS_ENB_COL_CLK_BUFF);
	if (ret < 0)
		return ret;

	apart->adev->ops->mem_clear(apart);
	aie_part_clear_core_regs(apart);
	ret = zynqmp_pm_aie_operation(node_id, apart->range.start.col,
				      apart->range.size.col,
				      XILINX_AIE_OPS_DIS_COL_CLK_BUFF);
	if (ret < 0)
		return ret;

	aie_resource_clear_all(&apart->cores_clk_state);

	return 0;
}

/**
 * aie_part_reset() - reset AI engine partition
 * @apart: AI engine partition
 * @return: 0 for success and negative value for failure
 *
 * This function will:
 * - gate all the columns
 * - reset AI engine partition columns
 * - ungate all the columns
 * - reset AI engine shims
 * - gate all the tiles in a partition.
 *
 * This function will not validate the partition, the caller will need to
 * provide a valid AI engine partition.
 */
int aie_part_reset(struct aie_partition *apart)
{
	u32 node_id = apart->adev->pm_node_id;
	int ret;

	ret = mutex_lock_interruptible(&apart->mlock);
	if (ret)
		return ret;

	/*
	 * Check if any AI engine memories or registers in the
	 * partition have been mapped. If yes, don't reset.
	 */
	if (aie_part_has_mem_mmapped(apart) ||
	    aie_part_has_regs_mmapped(apart)) {
		dev_err(&apart->dev,
			"failed to reset, there are mmapped memories or registers.\n");
		mutex_unlock(&apart->mlock);
		return -EBUSY;
	}

	/* Clear tiles in use bitmap and clock state bitmap */
	aie_resource_clear_all(&apart->tiles_inuse);
	aie_resource_clear_all(&apart->cores_clk_state);

	ret = zynqmp_pm_aie_operation(node_id, apart->range.start.col,
				      apart->range.size.col,
				      XILINX_AIE_OPS_DIS_COL_CLK_BUFF);
	if (ret < 0)
		goto exit;

	ret = zynqmp_pm_aie_operation(node_id, apart->range.start.col,
				      apart->range.size.col,
				      XILINX_AIE_OPS_COL_RST);
	if (ret < 0)
		goto exit;

	ret = zynqmp_pm_aie_operation(node_id, apart->range.start.col,
				      apart->range.size.col,
				      XILINX_AIE_OPS_ENB_COL_CLK_BUFF);
	if (ret < 0)
		goto exit;

	ret = zynqmp_pm_aie_operation(node_id, apart->range.start.col,
				      apart->range.size.col,
				      XILINX_AIE_OPS_SHIM_RST);
	if (ret < 0)
		goto exit;

	ret = zynqmp_pm_aie_operation(node_id, apart->range.start.col,
				      apart->range.size.col,
				      XILINX_AIE_OPS_DIS_COL_CLK_BUFF);
	if (ret < 0)
		goto exit;

	aie_part_clear_cached_events(apart);
	aie_part_rscmgr_reset(apart);

exit:
	mutex_unlock(&apart->mlock);

	return ret;
}

/**
 * aie_part_post_reinit() - AI engine partition has been re-initialized
 * @apart: AI engine partition
 * @return: 0 for success and negative value for failure
 *
 * This function will:
 * - scan which tiles are gated
 * - update memories and registers mapping
 *
 * This function will scan which tiles are gated, and update the memories and
 * registers setting. This function is called after the AI engine partition is
 * reconfigured with PDI outside the AI engine driver.
 */
int aie_part_post_reinit(struct aie_partition *apart)
{
	int ret;

	ret = mutex_lock_interruptible(&apart->mlock);
	if (ret)
		return ret;

	ret = aie_part_scan_clk_state(apart);
	mutex_unlock(&apart->mlock);
	if (ret) {
		dev_err(&apart->dev,
			"failed to scan clock states after reset is done.\n");
		return ret;
	}

	return 0;
}
