/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * ipu3_ipa_interface.h - Image Processing Algorithm interface for ipu3
 *
 * This file is auto-generated. Do not edit.
 */

#pragma once

#include <libcamera/ipa/core_ipa_interface.h>
#include <libcamera/ipa/ipa_interface.h>


#include <vector>

namespace libcamera {

namespace ipa {

namespace ipu3 {




enum class _IPU3Cmd {
	Exit = 0,
	Init = 1,
	Start = 2,
	Stop = 3,
	Configure = 4,
	MapBuffers = 5,
	UnmapBuffers = 6,
	QueueRequest = 7,
	FillParamsBuffer = 8,
	ProcessStatsBuffer = 9,
};

enum class _IPU3EventCmd {
	SetSensorControls = 1,
	ParamsBufferReady = 2,
	MetadataReady = 3,
};


struct IPAConfigInfo
{
public:
#ifndef __DOXYGEN__
	IPAConfigInfo()
	{
	}

	IPAConfigInfo(const IPACameraSensorInfo &_sensorInfo, const ControlInfoMap &_sensorControls, const ControlInfoMap &_lensControls, const Size &_bdsOutputSize, const Size &_iif)
		: sensorInfo(_sensorInfo), sensorControls(_sensorControls), lensControls(_lensControls), bdsOutputSize(_bdsOutputSize), iif(_iif)
	{
	}
#endif


	IPACameraSensorInfo sensorInfo;
	ControlInfoMap sensorControls;
	ControlInfoMap lensControls;
	Size bdsOutputSize;
	Size iif;
};

class IPAIPU3Interface : public IPAInterface
{
public:

	virtual int32_t init(
		const IPASettings &settings,
		const IPACameraSensorInfo &sensorInfo,
		const ControlInfoMap &sensorControls,
		ControlInfoMap *ipaControls) = 0;

	virtual int32_t start() = 0;

	virtual void stop() = 0;

	virtual int32_t configure(
		const IPAConfigInfo &configInfo,
		ControlInfoMap *ipaControls) = 0;

	virtual void mapBuffers(
		const std::vector<libcamera::IPABuffer> &buffers) = 0;

	virtual void unmapBuffers(
		const std::vector<uint32_t> &ids) = 0;

	virtual void queueRequest(
		const uint32_t frame,
		const ControlList &controls) = 0;

	virtual void fillParamsBuffer(
		const uint32_t frame,
		const uint32_t bufferId) = 0;

	virtual void processStatsBuffer(
		const uint32_t frame,
		const int64_t frameTimestamp,
		const uint32_t bufferId,
		const ControlList &sensorControls) = 0;

	Signal<uint32_t, const ControlList &, const ControlList &> setSensorControls;

	Signal<uint32_t> paramsBufferReady;

	Signal<uint32_t, const ControlList &> metadataReady;
};

} /* namespace ipu3 */

} /* namespace ipa */

} /* namespace libcamera */