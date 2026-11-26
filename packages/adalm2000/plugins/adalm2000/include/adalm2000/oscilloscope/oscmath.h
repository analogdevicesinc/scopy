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

#include <QList>

#include <algorithm>
#include <cmath>

namespace scopy::adalm2000::osc {

inline constexpr double MAX_SAMPLE_RATE = 100e6;
inline constexpr double MIN_SAMPLE_RATE = 1e3;

inline const QList<double> DEFAULT_HARDWARE_RATES{1e3, 1e4, 1e5, 1e6, 1e7, 1e8};

enum Digital
{
	DigitalOff,
	DigitalExternal,
	DigitalLogic,
};

// How an analog and a digital trigger combine. The logic analyzer only ORs.
enum Logic
{
	LogicOr,
	LogicAnd,
	LogicXor,
	LogicNor,
	LogicNand,
	LogicXnor,
};

struct Trigger
{
	bool enabled{true};
	// No trigger within the timeout: one untriggered buffer.
	bool autoMode{true};
	bool analog{true};
	int channel{0};
	// Indexes trigger::ANALOG_CONDITION_OPTIONS.
	int condition{0};
	double level{0.0};
	double hysteresis{0.05};
	int digital{DigitalOff};
	// Indexes trigger::DIGITAL_CONDITION_OPTIONS, for the external trigger in.
	int externalCondition{0};
	int logic{LogicOr};
	// Indexes trigger::OUT_SELECT_OPTIONS.
	int outSelect{0};
};

struct HwRate
{
	double rate{MAX_SAMPLE_RATE};
	int oversampling{1};
};

// The slowest hardware rate at or above `hz`, divided down by an integer oversampling.
inline HwRate hwRate(double hz, QList<double> available)
{
	if(available.isEmpty()) {
		available = DEFAULT_HARDWARE_RATES;
	}
	std::sort(available.begin(), available.end());
	for(double r : available) {
		if(r >= hz * (1 - 1e-9)) {
			return {r, std::max(1, static_cast<int>(std::lround(r / hz)))};
		}
	}
	return {available.last(), 1};
}

// The smallest 1-2-5 step at or above `v`.
inline double ceil125(double v)
{
	if(!(v > 0)) {
		return 0.0;
	}
	const double decade = std::pow(10.0, std::floor(std::log10(v)));
	for(double m : {1.0, 2.0, 5.0, 10.0}) {
		if(m * decade >= v * (1 - 1e-9)) {
			return m * decade;
		}
	}
	return 10.0 * decade;
}

// Autoset: the probe rates, fast first, and what a measured signal asks for.
inline const QList<double> AUTOSET_RATES{10e6, 100e3, 10e3};

inline constexpr double AUTOSET_PERIODS = 2.5;
inline constexpr double AUTOSET_MIN_VOLTS_PER_DIV = 10e-3;
inline constexpr double AUTOSET_DEFAULT_TIME_BASE = 1e-3;

inline double autosetTimeBase(double period, int divs)
{
	return period > 0 ? ceil125(AUTOSET_PERIODS * period / divs) : AUTOSET_DEFAULT_TIME_BASE;
}

// Peak to peak in 8 of the 10 divisions.
inline double autosetVoltsPerDiv(double peakPeak)
{
	return std::max(AUTOSET_MIN_VOLTS_PER_DIV, ceil125(peakPeak / 8.0));
}

} // namespace scopy::adalm2000::osc
