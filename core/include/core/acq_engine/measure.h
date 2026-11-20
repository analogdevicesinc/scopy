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

#include "scopy-core_export.h"

#include <QString>

#include <array>
#include <limits>

namespace scopy {
namespace acq {

// The oscilloscope measurements, in the order the old instruments listed them.
enum class Measure
{
	Period,
	Frequency,
	Min,
	Max,
	PeakPeak,
	Mean,
	CycleMean,
	Rms,
	CycleRms,
	AcRms,
	Area,
	CycleArea,
	Low,
	High,
	Amplitude,
	Middle,
	PosOver,
	NegOver,
	Rise,
	Fall,
	PosWidth,
	NegWidth,
	PosDuty,
	NegDuty,
};

inline constexpr int MEASURE_COUNT = static_cast<int>(Measure::NegDuty) + 1;

SCOPY_CORE_EXPORT QString measureName(Measure m);
// The script name, e.g. "peak_peak".
SCOPY_CORE_EXPORT QString measureId(Measure m);
SCOPY_CORE_EXPORT QString measureUnit(Measure m, const QString &yUnit = QStringLiteral("V"));

struct MeasureParams
{
	double sampleRate{1.0};
	// Measures [gateStart, gateEnd); gateEnd < 0 is the end of the data.
	int gateStart{0};
	int gateEnd{-1};
};

// Sample indices into the data, -1 when not found.
struct MeasureMarks
{
	int periodStart{-1};
	int periodEnd{-1};
	int lowRising{-1};
	int midRising{-1};
	int highRising{-1};
	int highFalling{-1};
	int midFalling{-1};
	int lowFalling{-1};
};

struct MeasureResult
{
	MeasureResult() { values.fill(std::numeric_limits<double>::quiet_NaN()); }

	// NaN where it could not be measured.
	std::array<double, MEASURE_COUNT> values;
	MeasureMarks marks;

	double value(Measure m) const { return values[static_cast<int>(m)]; }
};

SCOPY_CORE_EXPORT MeasureResult measure(const float *data, int n, const MeasureParams &p);

} // namespace acq
} // namespace scopy
