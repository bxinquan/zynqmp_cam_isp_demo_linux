/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2019, Raspberry Pi Ltd
 *
 * cam_helper_ar1335.cpp - camera information for ar1335 sensor
 */

#include <assert.h>

#include "cam_helper.h"

using namespace RPiController;

class CamHelperAr1335 : public CamHelper
{
public:
	CamHelperAr1335();
	uint32_t gainCode(double gain) const override;
	double gain(uint32_t gainCode) const override;
	void getDelays(int &exposureDelay, int &gainDelay,
		       int &vblankDelay, int &hblankDelay) const override;
	unsigned int hideFramesStartup() const override;
	unsigned int hideFramesModeSwitch() const override;
	unsigned int mistrustFramesStartup() const override;
	unsigned int mistrustFramesModeSwitch() const override;

private:
	/*
	 * Smallest difference between the frame length and integration time,
	 * in units of lines.
	 */
	static constexpr int frameIntegrationDiff = 4;
};

/*
 * AR1335 doesn't output metadata, so we have to use the "unicam parser" which
 * works by counting frames.
 */

CamHelperAr1335::CamHelperAr1335()
	: CamHelper({}, frameIntegrationDiff)
{
}

static const unsigned _gainTable[][2] = {
	{0x0010, 0x2010},
	{0x0011, 0x2011},
	{0x0012, 0x2012},
	{0x0013, 0x2013},
	{0x0014, 0x2014},
	{0x0015, 0x2015},
	{0x0016, 0x2016},
	{0x0017, 0x2017},
	{0x0018, 0x2018},
	{0x0019, 0x2019},
	{0x001A, 0x201A},
	{0x001B, 0x201B},
	{0x001C, 0x201C},
	{0x001D, 0x201D},
	{0x001E, 0x201E},
	{0x001F, 0x201F},
	{0x0020, 0x2020},
	{0x0022, 0x2021},
	{0x0024, 0x2022},
	{0x0026, 0x2023},
	{0x0028, 0x2024},
	{0x002A, 0x2025},
	{0x002C, 0x2026},
	{0x002E, 0x2027},
	{0x0030, 0x2028},
	{0x0032, 0x2029},
	{0x0034, 0x202A},
	{0x0036, 0x202B},
	{0x0038, 0x202C},
	{0x003A, 0x202D},
	{0x003C, 0x202E},
	{0x003E, 0x202F},
	{0x0040, 0x2030},
	{0x0044, 0x2031},
	{0x0048, 0x2032},
	{0x004C, 0x2033},
	{0x0050, 0x2034},
	{0x0054, 0x2035},
	{0x0058, 0x2036},
	{0x005C, 0x2037},
	{0x0060, 0x2038},
	{0x0064, 0x2039},
	{0x0068, 0x203A},
	{0x006C, 0x203B},
	{0x0070, 0x203C},
	{0x0074, 0x203D},
	{0x0075, 0x203E},
	{0x007C, 0x203F},
	{0x0080, 0x213F},
	{0x0084, 0x223F},
	{0x0088, 0x233F},
	{0x008C, 0x243F},
	{0x0090, 0x253F},
	{0x0094, 0x263F},
	{0x0098, 0x273F},
	{0x009C, 0x28BF},
	{0x00A0, 0x29BF},
	{0x00A4, 0x2ABF},
	{0x00A8, 0x2BBF},
	{0x00AC, 0x2CBF},
	{0x00B0, 0x2DBF},
	{0x00B4, 0x2EBF},
	{0x00B8, 0x2FBF},
	{0x00BC, 0x30BF},
	{0x00C0, 0x31BF},
	{0x00C4, 0x32BF},
	{0x00C8, 0x33BF},
	{0x00CC, 0x34BF},
	{0x00D0, 0x35BF},
	{0x00D4, 0x36BF},
	{0x00D8, 0x37BF},
	{0x00DC, 0x393F},
	{0x00E0, 0x3A3F},
	{0x00E4, 0x3B3F},
	{0x00E8, 0x3C3F},
	{0x00EC, 0x3D3F},
	{0x00F0, 0x3E3F},
	{0x00F4, 0x3F3F},
	{0x00F8, 0x403F},
	{0x00FC, 0x413F},
	{0x0100, 0x423F},
	{0x0104, 0x433F},
	{0x0108, 0x443F},
	{0x010C, 0x453F},
	{0x0110, 0x463F},
	{0x0114, 0x473F},
	{0x0118, 0x48BF},
	{0x011C, 0x49BF},
	{0x0120, 0x4ABF},
	{0x0124, 0x4BBF},
	{0x0128, 0x4CBF},
	{0x012C, 0x4DBF},
	{0x0130, 0x4EBF},
	{0x0134, 0x4FBF},
	{0x0138, 0x50BF},
	{0x013C, 0x51BF},
	{0x0140, 0x52BF},
	{0x0144, 0x53BF},
	{0x0148, 0x54BF},
	{0x014C, 0x55BF},
	{0x0150, 0x56BF},
	{0x0154, 0x57BF},
	{0x0158, 0x593F},
	{0x015C, 0x5A3F},
	{0x0160, 0x5B3F},
	{0x0164, 0x5C3F},
	{0x0168, 0x5D3F},
	{0x016C, 0x5E3F},
	{0x0170, 0x5F3F},
	{0x0174, 0x603F},
	{0x0178, 0x613F},
	{0x017C, 0x623F},
	{0x0180, 0x633F}
};

uint32_t CamHelperAr1335::gainCode(double gain) const
{
	unsigned u32Gain = gain * 16;
	unsigned index = 0;
	for (index = 0; index < sizeof(_gainTable)/sizeof(_gainTable[0]); index ++) {
		if (_gainTable[index][0] > u32Gain) {
			break;
		}
	}
	index = index > 0 ? index - 1 : 0;

	return static_cast<uint32_t>(_gainTable[index][1]);
}

double CamHelperAr1335::gain(uint32_t gainCode) const
{
	unsigned index = 0;
	for (index = 0; index < sizeof(_gainTable)/sizeof(_gainTable[0]); index ++) {
		if (_gainTable[index][1] > gainCode) {
			break;
		}
	}
	index = index > 0 ? index - 1 : 0;
	return static_cast<double>(_gainTable[index][0]) / 16.0;
}

void CamHelperAr1335::getDelays(int &exposureDelay, int &gainDelay,
				int &vblankDelay, int &hblankDelay) const
{
	/*
	 * We run this sensor in a mode where the gain delay is bumped up to
	 * 2. It seems to be the only way to make the delays "predictable".
	 */
	exposureDelay = 2;
	gainDelay = 1;
	vblankDelay = 2;
	hblankDelay = 2;
}

unsigned int CamHelperAr1335::hideFramesStartup() const
{
	/*
	 * On startup, we get a couple of under-exposed frames which
	 * we don't want shown.
	 */
	return 2;
}

unsigned int CamHelperAr1335::hideFramesModeSwitch() const
{
	/*
	 * After a mode switch, we get a couple of under-exposed frames which
	 * we don't want shown.
	 */
	return 2;
}

unsigned int CamHelperAr1335::mistrustFramesStartup() const
{
	/*
	 * First couple of frames are under-exposed and are no good for control
	 * algos.
	 */
	return 2;
}

unsigned int CamHelperAr1335::mistrustFramesModeSwitch() const
{
	/*
	 * First couple of frames are under-exposed even after a simple
	 * mode switch, and are no good for control algos.
	 */
	return 2;
}

static CamHelper *create()
{
	return new CamHelperAr1335();
}

static RegisterCamHelper reg("ar1335", &create);
