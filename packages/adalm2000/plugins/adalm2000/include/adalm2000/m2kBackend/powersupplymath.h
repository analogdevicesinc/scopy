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

#include <cstddef>
#include <deque>

// Header-only and free of Q_OBJECT on purpose: that keeps it out of AUTOMOC and
// therefore clear of the scopy::Command collision between iioutil/command.h and
// core/command.h.

namespace scopy::adalm2000::ps {

inline constexpr std::size_t AVERAGE_COUNT = 5;
inline constexpr int POLL_INTERVAL_MS = 200;

// libm2k's pushChannel() throws above 5 V in magnitude, so these are the
// hardware's own bounds, not a UI preference.
inline constexpr double POSITIVE_MIN = 0.0;
inline constexpr double POSITIVE_MAX = 5.0;
inline constexpr double NEGATIVE_MIN = -5.0;
inline constexpr double NEGATIVE_MAX = 0.0;
// The step in the attribute's range triple.
inline constexpr double VOLTAGE_STEP = 1.0;

inline constexpr int RATIO_MIN = 0;
inline constexpr int RATIO_MAX = 100;
inline constexpr int RATIO_DEFAULT = 100;

// The mean is taken over the samples ACTUALLY held, not over the window size,
// and that distinction is the whole point of this class. A fixed-window average
// (as voltmeterdsp.h's MovingAverage does) ramps up from zero for the first N-1
// samples, so a freshly cleared average would crawl toward the new voltage over
// five ticks instead of snapping to it.
class RailAverage
{
public:
	double push(double sample)
	{
		m_samples.push_back(sample);
		if(m_samples.size() > AVERAGE_COUNT) {
			m_samples.pop_front();
		}
		return value();
	}

	// Empty is reported as 0.0 rather than NaN: that is what the LCD shows before
	// the first tick.
	double value() const
	{
		if(m_samples.empty()) {
			return 0.0;
		}
		double sum = 0.0;
		for(double s : m_samples) {
			sum += s;
		}
		return sum / static_cast<double>(m_samples.size());
	}

	// Called on every set-value and every enable change for the affected rail.
	void clear() { m_samples.clear(); }

	std::size_t size() const { return m_samples.size(); }

private:
	std::deque<double> m_samples;
};

// Tracking: V- = -(ratio/100) * V+.
inline double trackingNegative(double positiveVolts, int ratioPercent)
{
	return -positiveVolts * static_cast<double>(ratioPercent) / 100.0;
}

} // namespace scopy::adalm2000::ps
