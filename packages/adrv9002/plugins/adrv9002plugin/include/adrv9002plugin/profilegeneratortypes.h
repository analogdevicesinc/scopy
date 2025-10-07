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

struct RxChannelConfig
{
	//  Enable channel
	bool enabled;
	// Enable high performance ADC, otherwise use low-power ADC
	bool adcHighPerformanceMode;
	// Enable ADC frequency offset correction
	bool freqOffsetCorrectionEnable;
	// Power mode of front-end analog filter Options are:
	// 	0 - Low power
	// 	1 - Medium power
	// 	2 - High power
	uint8_t analogFilterPowerMode;
	// Use second order (Biquad) analog filter, otherwise first order TIA is used
	bool analogFilterBiquad;
	// Front-end analog filter 1dB (Biquad) or 3 dB (TIA) bandwidth in Hz
	uint32_t analogFilterBandwidthHz;
	// Channel bandwidth of interest at ADC in Hz
	uint32_t channelBandwidthHz;
	// RX channel sample rate at digital interface
	uint32_t sampleRateHz;
	// Enable NCO to perform frequency translation
	bool ncoEnable;
	// NCO frequency in Hz
	int32_t ncoFrequencyHz;
	// RF port source used for channel Options are:
	//        0 - RX_A
	//        1 - RX_B
	uint8_t rfPort;
};

struct TxChannelConfig
{
	// Enable channel
	bool enabled;
	// Data rate at digital interface in Hz
	uint32_t sampleRateHz;
	// Enable DAC frequency offset correction
	bool freqOffsetCorrectionEnable;
	// Power mode of front-end analog filter Options are:
	// 	0 - Low power
	// 	1 - Medium power
	// 	2 - High power
	uint8_t analogFilterPowerMode;
	// Channel bandwidth of interest at DAC in Hz
	uint32_t channelBandwidthHz;
	// Enable observation path
	bool orxEnabled;
	// Set external loopback mode. Options are:
	//        0 - Disabled
	//        1 - Before PA
	//        2 - After PA
	uint8_t elbType;
};

struct ClockConfig
{
	// Template-based parameters (from iio-oscilloscope clock_config)
	uint32_t deviceClockFrequencyKhz;
	bool deviceClockOutputEnable;
	uint8_t deviceClockOutputDivider;
	//  Enable high performance PLL mode, otherwise low-power mode is used
	bool clockPllHighPerformanceEnable;
	//  PLL power mode. Options:
	//    0 = low power
	//    1 = medium performance
	//    2 = high performance
	uint8_t clockPllPowerMode;
	// Processor clock divider. Valid values are 1, 2, 4, 8, 16, 32, 64, 128, 256
	uint8_t processorClockDivider;
};

struct RadioConfig
{
	// SSI lanes to use Valid cases:
	//          1 (CMOS/LVDS)
	//          2 (LVDS)
	//          4 (CMOS)
	uint8_t ssi_lanes; // 1, 2, or 4 lanes
	// Use DDR mode at digital interface, false will use SDR
	bool ddr;
	// Use short strobe mode at digital interface, false will use long strobe
	bool shortStrobe;
	// Use LVDS mode at digital interface, false will use CMOS
	bool lvds;
	// ADC clock rate mode select. Options are:
	// 	1 = LOW
	// 	2 = MEDIUM
	// 	3 = HIGH
	uint8_t adcRateMode;
	//  Use FDD duplex mode, false will use TDD
	bool fdd;

	// Channel configurations
	RxChannelConfig rx_config[2]; // RX1, RX2
	TxChannelConfig tx_config[2]; // TX1, TX2

	// Clock configuration
	ClockConfig clk_config;
};

} // namespace scopy::adrv9002

#endif // PROFILEGENERATORTYPES_H
