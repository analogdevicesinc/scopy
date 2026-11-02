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
 *
 */

#pragma once

#include <QList>

#include <algorithm>
#include <cmath>

namespace scopy::adalm2000::spec {
inline constexpr double MAX_SAMPLE_RATE = 100e6;

inline constexpr double FULL_SCALE_COUNTS = 2048.0;

inline constexpr double MAX_STOP_HZ = MAX_SAMPLE_RATE / 2.0;
inline constexpr double MIN_STOP_HZ = 1e3;

inline constexpr double DEFAULT_START_HZ = 0.0;
inline constexpr double DEFAULT_STOP_HZ = MAX_STOP_HZ;
inline constexpr int DEFAULT_FFT_INDEX = 5;

inline constexpr double MIN_SPAN_BINS = 10.0;

inline int oversamplingFor(double stopHz)
{
	stopHz = std::clamp(stopHz, MIN_STOP_HZ, MAX_STOP_HZ);
	return std::max(1, static_cast<int>(std::floor(MAX_SAMPLE_RATE / (2.0 * stopHz))));
}

inline double sampleRateFor(double stopHz) { return MAX_SAMPLE_RATE / oversamplingFor(stopHz); }

inline QList<int> fftSizes()
{
	QList<int> out;
	for(int p = 8; p <= 18; ++p) {
		out.append(1 << p);
	}
	return out;
}

inline double rbw(double sampleRate, int fftSize) { return fftSize > 0 ? sampleRate / fftSize : 0.0; }

inline int bins(int fftSize) { return fftSize / 2 + 1; }

inline double minSpan(double sampleRate, int fftSize) { return MIN_SPAN_BINS * rbw(sampleRate, fftSize); }

struct Span
{
	double start{DEFAULT_START_HZ};
	double stop{DEFAULT_STOP_HZ};

	double center() const { return (start + stop) / 2.0; }
	double width() const { return stop - start; }
};

inline Span normalized(Span s, double minSpan)
{
	s.stop = std::clamp(s.stop, minSpan, MAX_STOP_HZ);
	s.start = std::clamp(s.start, 0.0, MAX_STOP_HZ - minSpan);
	if(s.stop - s.start < minSpan) {
		s.stop = std::min(MAX_STOP_HZ, s.start + minSpan);
		s.start = std::max(0.0, s.stop - minSpan);
	}
	return s;
}

inline Span withStart(Span s, double start, double minSpan)
{
	s.start = start;
	if(s.stop - s.start < minSpan) {
		s.stop = s.start + minSpan;
	}
	return normalized(s, minSpan);
}

inline Span withStop(Span s, double stop, double minSpan)
{
	s.stop = stop;
	if(s.stop - s.start < minSpan) {
		s.start = s.stop - minSpan;
	}
	return normalized(s, minSpan);
}

inline Span withCenter(Span s, double center, double minSpan)
{
	const double half = std::max(s.width(), minSpan) / 2.0;
	center = std::clamp(center, half, MAX_STOP_HZ - half);
	return normalized({center - half, center + half}, minSpan);
}

inline Span withWidth(Span s, double width, double minSpan)
{
	width = std::clamp(width, minSpan, MAX_STOP_HZ);
	const double half = width / 2.0;
	const double center = std::clamp(s.center(), half, MAX_STOP_HZ - half);
	return normalized({center - half, center + half}, minSpan);
}
} // namespace scopy::adalm2000::spec
