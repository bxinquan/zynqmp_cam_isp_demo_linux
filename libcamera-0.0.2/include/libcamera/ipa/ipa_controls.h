/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * ipa_controls.h - IPA Control handling
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
namespace libcamera {

extern "C" {
#endif

#define IPA_CONTROLS_FORMAT_VERSION	1

enum ipa_controls_id_map_type {
	IPA_CONTROL_ID_MAP_CONTROLS,
	IPA_CONTROL_ID_MAP_PROPERTIES,
	IPA_CONTROL_ID_MAP_V4L2,
};

struct ipa_controls_header {
	uint32_t version;
	uint32_t handle;
	uint32_t entries;
	uint32_t size;
	uint32_t data_offset;
	enum ipa_controls_id_map_type id_map_type;
	uint32_t reserved[2];
};

struct ipa_control_value_entry {
	uint32_t id;
	uint8_t type;
	uint8_t is_array;
	uint16_t count;
	uint32_t offset;
	uint32_t padding[1];
};

struct ipa_control_info_entry {
	uint32_t id;
	uint32_t type;
	uint32_t offset;
	uint32_t padding[1];
};

#ifdef __cplusplus
} /* namespace libcamera */

}
#endif
