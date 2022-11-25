/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * raspberrypi_ipa_interface.h - Image Processing Algorithm interface for raspberrypi
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

namespace RPi {



const uint32_t MaxLsGridSize = 0x8000;


enum class _RPiCmd {
	Exit = 0,
	Init = 1,
	Start = 2,
	Stop = 3,
	Configure = 4,
	MapBuffers = 5,
	UnmapBuffers = 6,
	SignalStatReady = 7,
	SignalQueueRequest = 8,
	SignalIspPrepare = 9,
};

enum class _RPiEventCmd {
	StatsMetadataComplete = 1,
	RunIsp = 2,
	EmbeddedComplete = 3,
	SetIspControls = 4,
	SetDelayedControls = 5,
};


enum BufferMask {
	MaskID = 65535,
	MaskStats = 65536,
	MaskEmbeddedData = 131072,
	MaskBayerData = 262144,
	MaskExternalBuffer = 1048576,
};

struct SensorConfig
{
public:
#ifndef __DOXYGEN__
	SensorConfig()
		: gainDelay(0), exposureDelay(0), vblankDelay(0), hblankDelay(0), sensorMetadata(0)
	{
	}

	SensorConfig(uint32_t _gainDelay, uint32_t _exposureDelay, uint32_t _vblankDelay, uint32_t _hblankDelay, uint32_t _sensorMetadata)
		: gainDelay(_gainDelay), exposureDelay(_exposureDelay), vblankDelay(_vblankDelay), hblankDelay(_hblankDelay), sensorMetadata(_sensorMetadata)
	{
	}
#endif


	uint32_t gainDelay;
	uint32_t exposureDelay;
	uint32_t vblankDelay;
	uint32_t hblankDelay;
	uint32_t sensorMetadata;
};

struct IPAInitResult
{
public:
#ifndef __DOXYGEN__
	IPAInitResult()
	{
	}

	IPAInitResult(const SensorConfig &_sensorConfig, const ControlInfoMap &_controlInfo)
		: sensorConfig(_sensorConfig), controlInfo(_controlInfo)
	{
	}
#endif


	SensorConfig sensorConfig;
	ControlInfoMap controlInfo;
};

struct ISPConfig
{
public:
#ifndef __DOXYGEN__
	ISPConfig()
		: embeddedBufferId(0), bayerBufferId(0), embeddedBufferPresent(0)
	{
	}

	ISPConfig(uint32_t _embeddedBufferId, uint32_t _bayerBufferId, bool _embeddedBufferPresent, const ControlList &_controls)
		: embeddedBufferId(_embeddedBufferId), bayerBufferId(_bayerBufferId), embeddedBufferPresent(_embeddedBufferPresent), controls(_controls)
	{
	}
#endif


	uint32_t embeddedBufferId;
	uint32_t bayerBufferId;
	bool embeddedBufferPresent;
	ControlList controls;
};

struct IPAConfig
{
public:
#ifndef __DOXYGEN__
	IPAConfig()
		: transform(0), lsTableHandle(-1)
	{
	}

	IPAConfig(uint32_t _transform, const SharedFD &_lsTableHandle)
		: transform(_transform), lsTableHandle(_lsTableHandle)
	{
	}
#endif


	uint32_t transform;
	SharedFD lsTableHandle;
};

struct IPAConfigResult
{
public:
#ifndef __DOXYGEN__
	IPAConfigResult()
		: modeSensitivity(0)
	{
	}

	IPAConfigResult(float _modeSensitivity, const ControlInfoMap &_controlInfo)
		: modeSensitivity(_modeSensitivity), controlInfo(_controlInfo)
	{
	}
#endif


	float modeSensitivity;
	ControlInfoMap controlInfo;
};

struct StartConfig
{
public:
#ifndef __DOXYGEN__
	StartConfig()
		: dropFrameCount(0), maxSensorFrameLengthMs(0)
	{
	}

	StartConfig(const ControlList &_controls, int32_t _dropFrameCount, uint32_t _maxSensorFrameLengthMs)
		: controls(_controls), dropFrameCount(_dropFrameCount), maxSensorFrameLengthMs(_maxSensorFrameLengthMs)
	{
	}
#endif


	ControlList controls;
	int32_t dropFrameCount;
	uint32_t maxSensorFrameLengthMs;
};

class IPARPiInterface : public IPAInterface
{
public:

	virtual int32_t init(
		const IPASettings &settings,
		IPAInitResult *result) = 0;

	virtual void start(
		const ControlList &controls,
		StartConfig *startConfig) = 0;

	virtual void stop() = 0;

	virtual int32_t configure(
		const IPACameraSensorInfo &sensorInfo,
		const std::map<uint32_t, libcamera::IPAStream> &streamConfig,
		const std::map<uint32_t, libcamera::ControlInfoMap> &entityControls,
		const IPAConfig &ipaConfig,
		ControlList *controls,
		IPAConfigResult *result) = 0;

	virtual void mapBuffers(
		const std::vector<libcamera::IPABuffer> &buffers) = 0;

	virtual void unmapBuffers(
		const std::vector<uint32_t> &ids) = 0;

	virtual void signalStatReady(
		const uint32_t bufferId) = 0;

	virtual void signalQueueRequest(
		const ControlList &controls) = 0;

	virtual void signalIspPrepare(
		const ISPConfig &data) = 0;

	Signal<uint32_t, const ControlList &> statsMetadataComplete;

	Signal<uint32_t> runIsp;

	Signal<uint32_t> embeddedComplete;

	Signal<const ControlList &> setIspControls;

	Signal<const ControlList &> setDelayedControls;
};

} /* namespace RPi */

} /* namespace ipa */

} /* namespace libcamera */