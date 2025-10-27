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

#include <profilegeneratorconstants.h>

#include <profilegeneratortypes.h>

using namespace scopy::adrv9002;

// Frequency table from iio-oscilloscope (exact Hz values)
const QStringList FrequencyTable::SAMPLE_RATES_HZ = {"1920000",	 "3840000",  "7680000",
						     "15360000", "30720000", "61440000"};

const QStringList FrequencyTable::BANDWIDTHS_HZ = {"1008000", "2700000", "4500000", "9000000", "18000000", "38000000"};

// RF Input options
const QStringList RFInputOptions::RX1_OPTIONS = {"Rx1A", "Rx1B"};

const QStringList RFInputOptions::RX2_OPTIONS = {"Rx2A", "Rx2B"};

// LTE defaults from lte_defaults() function (exact values)
const int LTEDefaults::SAMPLE_RATE_HZ = 61440000;
const int LTEDefaults::BANDWIDTH_HZ = 38000000;
const bool LTEDefaults::ENABLED = true;
const bool LTEDefaults::FREQ_OFFSET_CORRECTION = false;
const int LTEDefaults::RF_PORT = 0; // Maps to Rx1A/Rx2A

const QString LTEDefaults::SSI_INTERFACE = "LVDS";
const QString LTEDefaults::DUPLEX_MODE = "TDD";
const int LTEDefaults::SSI_LANES = 2;

const int LTEDefaults::DEVICE_CLOCK_FREQUENCY_KHZ = 38400;
const bool LTEDefaults::DEVICE_CLOCK_OUTPUT_ENABLE = true;
const int LTEDefaults::DEVICE_CLOCK_OUTPUT_DIVIDER = 2;

// Interface options
const QStringList IOOOptions::SSI_INTERFACE_OPTIONS = {"CMOS", "LVDS"};

const QStringList IOOOptions::DUPLEX_MODE_OPTIONS = {"TDD", "FDD"};

// Frequency table helper functions
int FrequencyTable::getIndexOfSampleRate(const QString &sampleRate) { return SAMPLE_RATES_HZ.indexOf(sampleRate); }

QString FrequencyTable::getBandwidthForSampleRate(const QString &sampleRate)
{
	int index = getIndexOfSampleRate(sampleRate);
	if(index >= 0 && index < BANDWIDTHS_HZ.size()) {
		return BANDWIDTHS_HZ[index];
	}
	return "38000000"; // Default fallback
}

QStringList FrequencyTable::getSampleRatesForSSILanes(int ssiLanes)
{
	// Based on iio-oscilloscope logic in profile_gen_config_set_LTE()
	if(ssiLanes == 1) {
		return QStringList(); // No options for 1 lane
	} else if(ssiLanes == 2) {
		return SAMPLE_RATES_HZ; // All 6 options for 2 lanes (LVDS)
	} else if(ssiLanes == 4) {
		return QStringList() << SAMPLE_RATES_HZ.first(); // Only first option for 4 lanes (CMOS)
	}
	return SAMPLE_RATES_HZ; // Default to all options
}

RadioConfig ProfileGeneratorConstants::lteDefaults()
{
	RadioConfig config;

	// TX Channel configurations
	for(int i = 0; i < 2; i++) {
		config.tx_config[i].enabled = true;
		config.tx_config[i].sampleRateHz = 61440000;
		config.tx_config[i].freqOffsetCorrectionEnable = false;
		config.tx_config[i].analogFilterPowerMode = 2; // High power
		config.tx_config[i].channelBandwidthHz = 38000000;
		config.tx_config[i].elbType = 0;
		config.tx_config[i].orxEnabled = false;
	}

	// RX Channel configurations
	for(int i = 0; i < 2; i++) {
		config.rx_config[i].enabled = true;
		config.rx_config[i].sampleRateHz = 61440000;
		config.rx_config[i].freqOffsetCorrectionEnable = false;
		config.rx_config[i].analogFilterPowerMode = 2; // High power
		config.rx_config[i].channelBandwidthHz = 38000000;
		config.rx_config[i].adcHighPerformanceMode = true;
		config.rx_config[i].analogFilterBiquad = false;		// from default cfg
		config.rx_config[i].analogFilterBandwidthHz = 18000000; // from default cfg
		config.rx_config[i].ncoEnable = false;
		config.rx_config[i].ncoFrequencyHz = 0;
		config.rx_config[i].rfPort = 0; // RxA ports
	}

	// Radio configuration
	config.ssi_lanes = 2;
	config.ddr = true; // needs logic
	config.shortStrobe = true;
	config.lvds = true;
	config.adcRateMode = 3; // HIGH
	config.fdd = false;	// TDD mode

	// Clock configuration
	config.clk_config.deviceClockFrequencyKhz = 38400;
	config.clk_config.deviceClockOutputEnable = true;
	config.clk_config.deviceClockOutputDivider = 2;
	config.clk_config.clockPllHighPerformanceEnable = false;
	config.clk_config.clockPllPowerMode = 2; // High power
	config.clk_config.processorClockDivider = 1;

	return config;
}

RadioConfig ProfileGeneratorConstants::lte_lvs3072MHz10()
{
	RadioConfig config;

	// TX Channel configurations
	for(int i = 0; i < 2; i++) {
		config.tx_config[i].enabled = true;
		config.tx_config[i].sampleRateHz = 30720000;
		config.tx_config[i].freqOffsetCorrectionEnable = false;
		config.tx_config[i].analogFilterPowerMode = 2; // High power
		config.tx_config[i].channelBandwidthHz = 18000000;
		config.tx_config[i].elbType = 2;
		config.tx_config[i].orxEnabled = true;
	}

	// RX Channel configurations
	for(int i = 0; i < 2; i++) {
		config.rx_config[i].enabled = true;
		config.rx_config[i].sampleRateHz = 30720000;
		config.rx_config[i].freqOffsetCorrectionEnable = false;
		config.rx_config[i].analogFilterPowerMode = 2; // High power
		config.rx_config[i].channelBandwidthHz = 18000000;
		config.rx_config[i].adcHighPerformanceMode = true;
		config.rx_config[i].analogFilterBiquad = false;	 // from default cfg
		config.rx_config[i].analogFilterBandwidthHz = 0; // TODO: not used?
		config.rx_config[i].ncoEnable = false;
		config.rx_config[i].ncoFrequencyHz = 0;
		config.rx_config[i].rfPort = 0; // RxA ports
	}

	// Radio configuration
	config.ssi_lanes = 2;
	config.ddr = true; // needs logic
	config.shortStrobe = true;
	config.lvds = true;
	config.adcRateMode = 3; // HIGH
	config.fdd = false;	// TDD mode

	// Clock configuration
	config.clk_config.deviceClockFrequencyKhz = 38400;
	config.clk_config.deviceClockOutputEnable = false;
	config.clk_config.deviceClockOutputDivider = 0; // TODO: not used?
	config.clk_config.clockPllHighPerformanceEnable = true;
	config.clk_config.clockPllPowerMode = 2; // High power
	config.clk_config.processorClockDivider = 1;
	config.clk_config.deviceClockOutputDivider = 0; // TODO: not used?
	config.clk_config.deviceClockOutputEnable = 0;

	return config;
}
