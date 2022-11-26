/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2019, Raspberry Pi Ltd
 *
 * lux.cpp - Lux control algorithm
 */
#include <math.h>

#include <linux/bcm2835-isp.h>

#include <libcamera/base/log.h>

#include "../device_status.h"
#include "../agc_status.h"

#include "lux.h"

using namespace RPiController;
using namespace libcamera;
using namespace std::literals::chrono_literals;

LOG_DEFINE_CATEGORY(RPiLux)

#define NAME "rpi.lux"

Lux::Lux(Controller *controller)
	: Algorithm(controller)
{
	/*
	 * Put in some defaults as there will be no meaningful values until
	 * Process has run.
	 */
	status_.aperture = 1.0;
	status_.lux = 400;
}

char const *Lux::name() const
{
	return NAME;
}

int Lux::read(const libcamera::YamlObject &params)
{
	auto value = params["reference_shutter_speed"].get<double>();
	if (!value)
		return -EINVAL;
	referenceShutterSpeed_ = *value * 1.0us;

	value = params["reference_gain"].get<double>();
	if (!value)
		return -EINVAL;
	referenceGain_ = *value;

	referenceAperture_ = params["reference_aperture"].get<double>(1.0);

	value = params["reference_Y"].get<double>();
	if (!value)
		return -EINVAL;
	referenceY_ = *value;

	value = params["reference_lux"].get<double>();
	if (!value)
		return -EINVAL;
	referenceLux_ = *value;

	currentAperture_ = referenceAperture_;
	return 0;
}

void Lux::setCurrentAperture(double aperture)
{
	currentAperture_ = aperture;
}

void Lux::prepare(Metadata *imageMetadata)
{
	std::unique_lock<std::mutex> lock(mutex_);
	imageMetadata->set("lux.status", status_);
}

void Lux::process(StatisticsPtr &stats, Metadata *imageMetadata)
{
	DeviceStatus deviceStatus;
	AgcStatus agcStatus;
	if (imageMetadata->get("agc.status", agcStatus) != 0) {
		LOG(RPiLux, Warning) << ": no agc status in metadata";
		agcStatus.digitalGain = 1.0;
	}
	if (imageMetadata->get("device.status", deviceStatus) == 0) {
		double currentGain = deviceStatus.analogueGain * agcStatus.digitalGain;
		double currentAperture = deviceStatus.aperture.value_or(currentAperture_);
		/* add .5 to reflect the mid-points of bins */
		double currentY = stats->ae.sum / (double)stats->ae.pix_cnt + .5;
		double gainRatio = referenceGain_ / currentGain;
		double shutterSpeedRatio =
			referenceShutterSpeed_ / deviceStatus.shutterSpeed;
		double apertureRatio = referenceAperture_ / currentAperture;
		double yRatio = currentY * (65536 / 1024) / referenceY_;
		double estimatedLux = shutterSpeedRatio * gainRatio *
				      apertureRatio * apertureRatio *
				      yRatio * referenceLux_;
		LuxStatus status;
		status.lux = estimatedLux;
		status.aperture = currentAperture;
		LOG(RPiLux, Debug) << ": estimated lux " << estimatedLux << ",  currentY " << currentY;
		{
			std::unique_lock<std::mutex> lock(mutex_);
			status_ = status;
		}
		/*
		 * Overwrite the metadata here as well, so that downstream
		 * algorithms get the latest value.
		 */
		imageMetadata->set("lux.status", status);
	} else
		LOG(RPiLux, Warning) << ": no device metadata";
}

/* Register algorithm with the system. */
static Algorithm *create(Controller *controller)
{
	return (Algorithm *)new Lux(controller);
}
static RegisterAlgorithm reg(NAME, &create);
