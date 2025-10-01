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

#ifndef PROFILEGENERATORCONSTANTS_H
#define PROFILEGENERATORCONSTANTS_H

#include <QString>
#include <QStringList>

namespace scopy::adrv9002 {

// Frequency table from iio-oscilloscope (exact Hz values)
class FrequencyTable
{
public:
	// Sample rates in Hz (from freq_table[0])
	static const QStringList SAMPLE_RATES_HZ;
	// Corresponding bandwidths in Hz (from freq_table[1])
	static const QStringList BANDWIDTHS_HZ;

	// Helper functions
	static int getIndexOfSampleRate(const QString &sampleRate);
	static QString getBandwidthForSampleRate(const QString &sampleRate);
	static QStringList getSampleRatesForSSILanes(int ssiLanes);
};

// RF Input options (descriptive names from screenshots)
class RFInputOptions
{
public:
	static const QStringList RX1_OPTIONS;
	static const QStringList RX2_OPTIONS;
};

// LTE defaults from lte_defaults() function
struct LTEDefaults
{
	// Channel defaults (Hz values)
	static const int SAMPLE_RATE_HZ;
	static const int BANDWIDTH_HZ;
	static const bool ENABLED;
	static const bool FREQ_OFFSET_CORRECTION;
	static const int RF_PORT;

	// Radio defaults
	static const QString SSI_INTERFACE;
	static const QString DUPLEX_MODE;
	static const int SSI_LANES;

	// Clock defaults
	static const int DEVICE_CLOCK_FREQUENCY_KHZ;
	static const bool DEVICE_CLOCK_OUTPUT_ENABLE;
	static const int DEVICE_CLOCK_OUTPUT_DIVIDER;
};

// Interface options
class IOOOptions
{
public:
	static const QStringList SSI_INTERFACE_OPTIONS;
	static const QStringList DUPLEX_MODE_OPTIONS;
};

} // namespace scopy::adrv9002

#endif // PROFILEGENERATORCONSTANTS_H