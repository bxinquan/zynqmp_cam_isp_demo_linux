/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * control_ids.h - Control ID list
 *
 * This file is auto-generated. Do not edit.
 */

#pragma once

#include <array>
#include <stdint.h>

#include <libcamera/controls.h>

namespace libcamera {

namespace controls {

enum {
	AE_ENABLE = 1,
	AE_LOCKED = 2,
	AE_METERING_MODE = 3,
	AE_CONSTRAINT_MODE = 4,
	AE_EXPOSURE_MODE = 5,
	EXPOSURE_VALUE = 6,
	EXPOSURE_TIME = 7,
	ANALOGUE_GAIN = 8,
	BRIGHTNESS = 9,
	CONTRAST = 10,
	LUX = 11,
	AWB_ENABLE = 12,
	AWB_MODE = 13,
	AWB_LOCKED = 14,
	COLOUR_GAINS = 15,
	COLOUR_TEMPERATURE = 16,
	SATURATION = 17,
	SENSOR_BLACK_LEVELS = 18,
	SHARPNESS = 19,
	FOCUS_FO_M = 20,
	COLOUR_CORRECTION_MATRIX = 21,
	SCALER_CROP = 22,
	DIGITAL_GAIN = 23,
	FRAME_DURATION = 24,
	FRAME_DURATION_LIMITS = 25,
	SENSOR_TEMPERATURE = 26,
	SENSOR_TIMESTAMP = 27,
	AF_MODE = 28,
	AF_RANGE = 29,
	AF_SPEED = 30,
	AF_METERING = 31,
	AF_WINDOWS = 32,
	AF_TRIGGER = 33,
	AF_PAUSE = 34,
	LENS_POSITION = 35,
	AF_STATE = 36,
	AF_PAUSE_STATE = 37,
	AE_PRECAPTURE_TRIGGER = 38,
	NOISE_REDUCTION_MODE = 39,
	COLOR_CORRECTION_ABERRATION_MODE = 40,
	AE_STATE = 41,
	AWB_STATE = 42,
	SENSOR_ROLLING_SHUTTER_SKEW = 43,
	LENS_SHADING_MAP_MODE = 44,
	SCENE_FLICKER = 45,
	PIPELINE_DEPTH = 46,
	MAX_LATENCY = 47,
	TEST_PATTERN_MODE = 48,
};

extern const Control<bool> AeEnable;
extern const Control<bool> AeLocked;
enum AeMeteringModeEnum {
	MeteringCentreWeighted = 0,
	MeteringSpot = 1,
	MeteringMatrix = 2,
	MeteringCustom = 3,
};
extern const std::array<const ControlValue, 4> AeMeteringModeValues;
extern const Control<int32_t> AeMeteringMode;
enum AeConstraintModeEnum {
	ConstraintNormal = 0,
	ConstraintHighlight = 1,
	ConstraintShadows = 2,
	ConstraintCustom = 3,
};
extern const std::array<const ControlValue, 4> AeConstraintModeValues;
extern const Control<int32_t> AeConstraintMode;
enum AeExposureModeEnum {
	ExposureNormal = 0,
	ExposureShort = 1,
	ExposureLong = 2,
	ExposureCustom = 3,
};
extern const std::array<const ControlValue, 4> AeExposureModeValues;
extern const Control<int32_t> AeExposureMode;
extern const Control<float> ExposureValue;
extern const Control<int32_t> ExposureTime;
extern const Control<float> AnalogueGain;
extern const Control<float> Brightness;
extern const Control<float> Contrast;
extern const Control<float> Lux;
extern const Control<bool> AwbEnable;
enum AwbModeEnum {
	AwbAuto = 0,
	AwbIncandescent = 1,
	AwbTungsten = 2,
	AwbFluorescent = 3,
	AwbIndoor = 4,
	AwbDaylight = 5,
	AwbCloudy = 6,
	AwbCustom = 7,
};
extern const std::array<const ControlValue, 8> AwbModeValues;
extern const Control<int32_t> AwbMode;
extern const Control<bool> AwbLocked;
extern const Control<Span<const float, 2>> ColourGains;
extern const Control<int32_t> ColourTemperature;
extern const Control<float> Saturation;
extern const Control<Span<const int32_t, 4>> SensorBlackLevels;
extern const Control<float> Sharpness;
extern const Control<int32_t> FocusFoM;
extern const Control<Span<const float, 9>> ColourCorrectionMatrix;
extern const Control<Rectangle> ScalerCrop;
extern const Control<float> DigitalGain;
extern const Control<int64_t> FrameDuration;
extern const Control<Span<const int64_t, 2>> FrameDurationLimits;
extern const Control<float> SensorTemperature;
extern const Control<int64_t> SensorTimestamp;
enum AfModeEnum {
	AfModeManual = 0,
	AfModeAuto = 1,
	AfModeContinuous = 2,
};
extern const std::array<const ControlValue, 3> AfModeValues;
extern const Control<int32_t> AfMode;
enum AfRangeEnum {
	AfRangeNormal = 0,
	AfRangeMacro = 1,
	AfRangeFull = 2,
};
extern const std::array<const ControlValue, 3> AfRangeValues;
extern const Control<int32_t> AfRange;
enum AfSpeedEnum {
	AfSpeedNormal = 0,
	AfSpeedFast = 1,
};
extern const std::array<const ControlValue, 2> AfSpeedValues;
extern const Control<int32_t> AfSpeed;
enum AfMeteringEnum {
	AfMeteringAuto = 0,
	AfMeteringWindows = 1,
};
extern const std::array<const ControlValue, 2> AfMeteringValues;
extern const Control<int32_t> AfMetering;
extern const Control<Span<const Rectangle>> AfWindows;
enum AfTriggerEnum {
	AfTriggerStart = 0,
	AfTriggerCancel = 1,
};
extern const std::array<const ControlValue, 2> AfTriggerValues;
extern const Control<int32_t> AfTrigger;
enum AfPauseEnum {
	AfPauseImmediate = 0,
	AfPauseDeferred = 1,
	AfPauseResume = 2,
};
extern const std::array<const ControlValue, 3> AfPauseValues;
extern const Control<int32_t> AfPause;
extern const Control<float> LensPosition;
enum AfStateEnum {
	AfStateIdle = 0,
	AfStateScanning = 1,
	AfStateFocused = 2,
	AfStateFailed = 3,
};
extern const std::array<const ControlValue, 4> AfStateValues;
extern const Control<int32_t> AfState;
enum AfPauseStateEnum {
	AfPauseStateRunning = 0,
	AfPauseStatePausing = 1,
	AfPauseStatePaused = 2,
};
extern const std::array<const ControlValue, 3> AfPauseStateValues;
extern const Control<int32_t> AfPauseState;

extern const ControlIdMap controls;

namespace draft {

enum AePrecaptureTriggerEnum {
	AePrecaptureTriggerIdle = 0,
	AePrecaptureTriggerStart = 1,
	AePrecaptureTriggerCancel = 2,
};
extern const std::array<const ControlValue, 3> AePrecaptureTriggerValues;
extern const Control<int32_t> AePrecaptureTrigger;
enum NoiseReductionModeEnum {
	NoiseReductionModeOff = 0,
	NoiseReductionModeFast = 1,
	NoiseReductionModeHighQuality = 2,
	NoiseReductionModeMinimal = 3,
	NoiseReductionModeZSL = 4,
};
extern const std::array<const ControlValue, 5> NoiseReductionModeValues;
extern const Control<int32_t> NoiseReductionMode;
enum ColorCorrectionAberrationModeEnum {
	ColorCorrectionAberrationOff = 0,
	ColorCorrectionAberrationFast = 1,
	ColorCorrectionAberrationHighQuality = 2,
};
extern const std::array<const ControlValue, 3> ColorCorrectionAberrationModeValues;
extern const Control<int32_t> ColorCorrectionAberrationMode;
enum AeStateEnum {
	AeStateInactive = 0,
	AeStateSearching = 1,
	AeStateConverged = 2,
	AeStateLocked = 3,
	AeStateFlashRequired = 4,
	AeStatePrecapture = 5,
};
extern const std::array<const ControlValue, 6> AeStateValues;
extern const Control<int32_t> AeState;
enum AwbStateEnum {
	AwbStateInactive = 0,
	AwbStateSearching = 1,
	AwbConverged = 2,
	AwbLocked = 3,
};
extern const std::array<const ControlValue, 4> AwbStateValues;
extern const Control<int32_t> AwbState;
extern const Control<int64_t> SensorRollingShutterSkew;
enum LensShadingMapModeEnum {
	LensShadingMapModeOff = 0,
	LensShadingMapModeOn = 1,
};
extern const std::array<const ControlValue, 2> LensShadingMapModeValues;
extern const Control<int32_t> LensShadingMapMode;
enum SceneFlickerEnum {
	SceneFickerOff = 0,
	SceneFicker50Hz = 1,
	SceneFicker60Hz = 2,
};
extern const std::array<const ControlValue, 3> SceneFlickerValues;
extern const Control<int32_t> SceneFlicker;
extern const Control<int32_t> PipelineDepth;
extern const Control<int32_t> MaxLatency;
enum TestPatternModeEnum {
	TestPatternModeOff = 0,
	TestPatternModeSolidColor = 1,
	TestPatternModeColorBars = 2,
	TestPatternModeColorBarsFadeToGray = 3,
	TestPatternModePn9 = 4,
	TestPatternModeCustom1 = 256,
};
extern const std::array<const ControlValue, 6> TestPatternModeValues;
extern const Control<int32_t> TestPatternMode;

} /* namespace draft */

} /* namespace controls */

} /* namespace libcamera */
