/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * core_ipa_interface.h - libcamera core definitions for Image Processing Algorithms
 *
 * This file is auto-generated. Do not edit.
 */

#pragma once


#include <vector>

#include <libcamera/ipa/ipa_interface.h>

namespace libcamera {





struct IPACameraSensorInfo
{
public:
#ifndef __DOXYGEN__
	IPACameraSensorInfo()
		: bitsPerPixel(0), pixelRate(0), minLineLength(0), maxLineLength(0), minFrameLength(0), maxFrameLength(0)
	{
	}

	IPACameraSensorInfo(const std::string &_model, uint32_t _bitsPerPixel, const Size &_activeAreaSize, const Rectangle &_analogCrop, const Size &_outputSize, uint64_t _pixelRate, uint32_t _minLineLength, uint32_t _maxLineLength, uint32_t _minFrameLength, uint32_t _maxFrameLength)
		: model(_model), bitsPerPixel(_bitsPerPixel), activeAreaSize(_activeAreaSize), analogCrop(_analogCrop), outputSize(_outputSize), pixelRate(_pixelRate), minLineLength(_minLineLength), maxLineLength(_maxLineLength), minFrameLength(_minFrameLength), maxFrameLength(_maxFrameLength)
	{
	}
#endif


	std::string model;
	uint32_t bitsPerPixel;
	Size activeAreaSize;
	Rectangle analogCrop;
	Size outputSize;
	uint64_t pixelRate;
	uint32_t minLineLength;
	uint32_t maxLineLength;
	uint32_t minFrameLength;
	uint32_t maxFrameLength;
};

struct IPABuffer
{
public:
#ifndef __DOXYGEN__
	IPABuffer()
		: id(0)
	{
	}

	IPABuffer(uint32_t _id, const std::vector<FrameBuffer::Plane> &_planes)
		: id(_id), planes(_planes)
	{
	}
#endif


	uint32_t id;
	std::vector<FrameBuffer::Plane> planes;
};

struct IPASettings
{
public:
#ifndef __DOXYGEN__
	IPASettings()
	{
	}

	IPASettings(const std::string &_configurationFile, const std::string &_sensorModel)
		: configurationFile(_configurationFile), sensorModel(_sensorModel)
	{
	}
#endif


	std::string configurationFile;
	std::string sensorModel;
};

struct IPAStream
{
public:
#ifndef __DOXYGEN__
	IPAStream()
		: pixelFormat(0)
	{
	}

	IPAStream(uint32_t _pixelFormat, const Size &_size)
		: pixelFormat(_pixelFormat), size(_size)
	{
	}
#endif


	uint32_t pixelFormat;
	Size size;
};


} /* namespace libcamera */