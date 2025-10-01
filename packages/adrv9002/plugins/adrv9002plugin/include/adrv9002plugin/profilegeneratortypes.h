/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef PROFILEGENERATORTYPES_H
#define PROFILEGENERATORTYPES_H

#include <QString>
#include <cstdint>

namespace scopy::adrv9002 {

enum ChannelType
{
	CHANNEL_RX1,
	CHANNEL_RX2,
	CHANNEL_TX1,
	CHANNEL_TX2
};

// Simplified for iio-oscilloscope compatibility - UpdateBatcher system removed

struct ChannelConfig
{
	bool enabled;
	bool freqOffsetCorrection;
	QString bandwidth;
	QString sampleRate;
	QString rfInput; // For RX only
};

struct OrxConfig
{
	bool enabled;
};

struct RadioConfig
{
	bool fdd;	   // Duplex mode (false=TDD, true=FDD)
	bool lvds;	   // SSI interface (true=LVDS, false=CMOS)
	uint8_t ssi_lanes; // 1, 2, or 4 lanes

	ChannelConfig rx_config[2]; // RX1, RX2
	ChannelConfig tx_config[2]; // TX1, TX2
	bool orx_enabled[2];	    // ORX1, ORX2
};

} // namespace scopy::adrv9002

#endif // PROFILEGENERATORTYPES_H