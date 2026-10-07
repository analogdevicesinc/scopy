/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdint>

// Header-only and free of Q_OBJECT on purpose: that keeps it out of AUTOMOC and
// therefore clear of the scopy::Command collision between iioutil/command.h and
// core/command.h.

namespace scopy::adalm2000::dio {

inline constexpr int PIN_COUNT = 16;
inline constexpr int PINS_PER_GROUP = 8;
inline constexpr int GROUP_COUNT = PIN_COUNT / PINS_PER_GROUP;
inline constexpr int GROUP_VALUE_MAX = (1 << PINS_PER_GROUP) - 1;

// One tick costs PIN_COUNT separate getValueRaw() calls -- libm2k has no bulk
// read -- so this interval trades input-refresh latency against link traffic.
inline constexpr int POLL_INTERVAL_MS = 500;

// Order is load-bearing. SwitchAttrUi labels its CustomSwitch from these and
// checks the switch iff the current value equals options[0], so index 0 is both
// the left label and the checked state -- it has to be the ACTIVE sense ("out",
// "1"). Reversing either pair silently inverts every switch in the tool.
inline constexpr const char *DIRECTION_OPTIONS[2] = {"out", "in"};
inline constexpr const char *VALUE_OPTIONS[2] = {"1", "0"};

// `direction` and `gpo` hold the user's INTENT; the hardware only holds
// intent & outputEnabled, and Run flushes the difference. Shadowing the intent
// is what lets a pin be set to output BEFORE Run without the control snapping
// back to "in" on the read-after-write settle.
struct PinState
{
	uint16_t direction = 0; // 1 == output, matching DIO_OUTPUT
	uint16_t gpo = 0;	// buffered output levels
	bool outputEnabled = false;
};

inline constexpr bool validPin(int pin) { return pin >= 0 && pin < PIN_COUNT; }

inline constexpr bool validGroup(int group) { return group >= 0 && group < GROUP_COUNT; }

inline constexpr bool bit(uint16_t mask, int pin) { return validPin(pin) ? ((mask >> pin) & 1u) != 0u : false; }

inline constexpr uint16_t withBit(uint16_t mask, int pin, bool value)
{
	if(!validPin(pin)) {
		return mask;
	}
	const uint16_t m = static_cast<uint16_t>(1u << pin);
	return static_cast<uint16_t>(value ? (mask | m) : (mask & static_cast<uint16_t>(~m)));
}

inline constexpr int groupBase(int group) { return validGroup(group) ? group * PINS_PER_GROUP : 0; }

// LSB-first: bit 0 is the lowest pin of the group. That is what the doc's
// "grouped value should be the binary value of channels 8-15" means.
inline constexpr uint8_t groupValue(uint16_t mask, int group)
{
	if(!validGroup(group)) {
		return 0;
	}
	return static_cast<uint8_t>((mask >> groupBase(group)) & 0xffu);
}

inline constexpr uint16_t withGroupValue(uint16_t mask, int group, uint8_t value)
{
	if(!validGroup(group)) {
		return mask;
	}
	const int base = groupBase(group);
	const uint16_t clear = static_cast<uint16_t>(~(0xffu << base));
	return static_cast<uint16_t>((mask & clear) | (static_cast<uint16_t>(value) << base));
}

// The group's direction control is a single two-state switch over 8 pins, so a
// mixed group (reachable by editing one pin on the Individual page) has no
// faithful representation. Reporting "output" only for a wholly-output group is
// the reading that cannot mislabel an undriven pin as driven.
inline constexpr bool groupIsOutput(uint16_t direction, int group)
{
	if(!validGroup(group)) {
		return false;
	}
	return groupValue(direction, group) == 0xffu;
}

// An output pin whose read-back disagrees with the level being driven: either
// shorted or something external is fighting the driver. Legacy also suppresses
// this for pins locked by the Pattern Generator; that lock is not implemented in
// this port, so when the Pattern Generator lands a locked() term belongs here.
inline constexpr bool isShorted(uint16_t gpi, const PinState &state, int pin)
{
	if(!validPin(pin) || !state.outputEnabled) {
		return false;
	}
	if(!bit(state.direction, pin)) {
		return false;
	}
	return bit(gpi, pin) != bit(state.gpo, pin);
}

} // namespace scopy::adalm2000::dio
