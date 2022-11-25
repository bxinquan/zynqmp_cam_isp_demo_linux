/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * rkisp1_ipa_interface.h - Image Processing Algorithm interface for rkisp1
 *
 * This file is auto-generated. Do not edit.
 */

#pragma once

#include <libcamera/ipa/core_ipa_interface.h>
#include <libcamera/ipa/ipa_interface.h>

#include <map>
#include <vector>

namespace libcamera {

namespace ipa {

namespace rkisp1 {




enum class _RkISP1Cmd {
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

enum class _RkISP1EventCmd {
	ParamsBufferReady = 1,
	SetSensorControls = 2,
	MetadataReady = 3,
};


class IPARkISP1Interface : public IPAInterface
{
public:

	virtual int32_t init(
		const IPASettings &settings,
		const uint32_t hwRevision,
		ControlInfoMap *ipaControls) = 0;

	virtual int32_t start() = 0;

	virtual void stop() = 0;

	virtual int32_t configure(
		const IPACameraSensorInfo &sensorInfo,
		const std::map<uint32_t, libcamera::IPAStream> &streamConfig,
		const std::map<uint32_t, libcamera::ControlInfoMap> &entityControls) = 0;

	virtual void mapBuffers(
		const std::vector<libcamera::IPABuffer> &buffers) = 0;

	virtual void unmapBuffers(
		const std::vector<uint32_t> &ids) = 0;

	virtual void queueRequest(
		const uint32_t frame,
		const ControlList &reqControls) = 0;

	virtual void fillParamsBuffer(
		const uint32_t frame,
		const uint32_t bufferId) = 0;

	virtual void processStatsBuffer(
		const uint32_t frame,
		const uint32_t bufferId,
		const ControlList &sensorControls) = 0;

	Signal<uint32_t> paramsBufferReady;

	Signal<uint32_t, const ControlList &> setSensorControls;

	Signal<uint32_t, const ControlList &> metadataReady;
};

} /* namespace rkisp1 */

} /* namespace ipa */

} /* namespace libcamera */