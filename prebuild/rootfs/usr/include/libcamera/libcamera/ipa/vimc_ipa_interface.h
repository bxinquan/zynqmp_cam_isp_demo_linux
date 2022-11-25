/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * vimc_ipa_interface.h - Image Processing Algorithm interface for vimc
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

namespace vimc {



const std::string VimcIPAFIFOPath = "/tmp/libcamera_ipa_vimc_fifo";


enum class _VimcCmd {
	Exit = 0,
	Init = 1,
	Configure = 2,
	Start = 3,
	Stop = 4,
	MapBuffers = 5,
	UnmapBuffers = 6,
	QueueRequest = 7,
	FillParamsBuffer = 8,
};

enum class _VimcEventCmd {
	ParamsBufferReady = 1,
};


enum IPAOperationCode {
	IPAOperationNone = 0,
	IPAOperationInit = 1,
	IPAOperationStart = 2,
	IPAOperationStop = 3,
};

enum class TestFlag {
	Flag1 = 1,
	Flag2 = 2,
	Flag3 = 4,
	Flag4 = 8,
};

class IPAVimcInterface : public IPAInterface
{
public:

	virtual int32_t init(
		const IPASettings &settings,
		const IPAOperationCode code,
		const Flags<ipa::vimc::TestFlag> inFlags,
		Flags<ipa::vimc::TestFlag> *outFlags) = 0;

	virtual int32_t configure(
		const IPACameraSensorInfo &sensorInfo,
		const std::map<uint32_t, libcamera::IPAStream> &streamConfig,
		const std::map<uint32_t, libcamera::ControlInfoMap> &entityControls) = 0;

	virtual int32_t start() = 0;

	virtual void stop() = 0;

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

	Signal<uint32_t, const Flags<ipa::vimc::TestFlag>> paramsBufferReady;
};

} /* namespace vimc */

} /* namespace ipa */

} /* namespace libcamera */