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
#include <QtGlobal>

#include <algorithm>
#include <cmath>

// One horizontal state seen two ways: samples (rate, buffer, plot window, delay) and time
// base (time per division, position).
namespace scopy::adalm2000::sweep {

struct Sweep
{
	double sampleRate{1e6};
	int bufferSize{1000};
	int plotWindow{1000};
	// Samples from the trigger to the left edge of the window.
	int delay{0};

	double span() const { return plotWindow / sampleRate; }
	double timeBase(int divs) const { return span() / divs; }
	// The time at the center of the window, relative to the trigger.
	double position() const { return (delay + plotWindow / 2.0) / sampleRate; }

	bool operator==(const Sweep &o) const
	{
		return sampleRate == o.sampleRate && bufferSize == o.bufferSize && plotWindow == o.plotWindow &&
			delay == o.delay;
	}
};

struct Limits
{
	// Ascending. The samples tab takes any rate in between, the time base only these.
	QList<double> rates;
	int minSamples{100};
	int maxBuffer{1 << 20};
	// Above maxBuffer the tool streams.
	int maxWindow{1 << 20};
	// The most samples the time base asks for.
	int timeBaseSamples{1 << 16};
	int delayMin{-8192};
	int delayMax{8191};
	// The trigger stays inside the buffer: no delay below -bufferSize.
	bool triggerInBuffer{false};
	// Time bases from here up stream, in buffers a tenth of the window; 0 never does.
	double streamTimeBase{0.0};
};

inline constexpr int STREAM_BUFFERS_PER_WINDOW = 10;

inline QList<double> series125(double min, double max)
{
	QList<double> out;
	for(double decade = 1.0; decade <= max; decade *= 10.0) {
		for(double m : {1.0, 2.0, 5.0}) {
			const double r = m * decade;
			if(r >= min && r <= max) {
				out.append(r);
			}
		}
	}
	return out;
}

// The M2K moves samples in groups of four, so every count is a multiple of 4 within [lo, hi].
inline int align4(long long n, long long lo, long long hi)
{
	const long long q = n >= 0 ? (n + 2) / 4 : -((-n + 1) / 4);
	const long long first = lo >= 0 ? (lo + 3) / 4 : -(-lo / 4);
	const long long last = hi >= 0 ? hi / 4 : -((-hi + 3) / 4);
	return static_cast<int>(4 * std::clamp(q, first, last));
}

inline int delayMin(const Sweep &s, const Limits &l)
{
	return l.triggerInBuffer ? std::max(l.delayMin, -s.bufferSize) : l.delayMin;
}

inline Sweep normalized(Sweep s, const Limits &l)
{
	s.sampleRate = std::clamp(s.sampleRate, l.rates.first(), l.rates.last());
	s.bufferSize = align4(s.bufferSize, l.minSamples, l.maxBuffer);
	s.plotWindow = align4(s.plotWindow, s.bufferSize, std::max(l.maxWindow, s.bufferSize));
	s.delay = align4(s.delay, delayMin(s, l), l.delayMax);
	return s;
}

// The trigger keeps its place on the screen.
inline Sweep withWindow(Sweep s, int buffer, int window, const Limits &l)
{
	const double frac = s.plotWindow > 0 ? double(s.delay) / s.plotWindow : 0.0;
	s.bufferSize = buffer;
	s.plotWindow = window;
	s = normalized(s, l);
	s.delay = align4(std::lround(frac * s.plotWindow), delayMin(s, l), l.delayMax);
	return s;
}

// The window follows the buffer, unless it streams.
inline Sweep withBuffer(const Sweep &s, int buffer, const Limits &l)
{
	const bool streaming = s.plotWindow > s.bufferSize;
	return withWindow(s, buffer, streaming ? std::max(s.plotWindow, buffer) : buffer, l);
}

// The highest listed rate whose window fits the time base budget and whose delay the
// hardware takes; the lowest one when none does.
inline Sweep fromTimeBase(double secPerDiv, double position, int divs, const Limits &l)
{
	const double span = secPerDiv * divs;
	if(l.streamTimeBase > 0 && secPerDiv >= l.streamTimeBase * (1 - 1e-9)) {
		double rate = l.rates.first();
		for(double r : l.rates) {
			if(span * r <= l.timeBaseSamples) {
				rate = r;
			}
		}
		Sweep s;
		s.sampleRate = rate;
		s.plotWindow =
			static_cast<int>(std::clamp(std::llround(span * rate), static_cast<long long>(l.minSamples),
						    static_cast<long long>(l.maxWindow)));
		s.bufferSize = std::clamp(s.plotWindow / STREAM_BUFFERS_PER_WINDOW, l.minSamples, l.maxBuffer);
		s.delay = 0;
		return normalized(s, l);
	}
	auto window = [&](double rate) {
		return static_cast<int>(std::clamp(std::llround(span * rate), static_cast<long long>(l.minSamples),
						   static_cast<long long>(l.timeBaseSamples)));
	};
	auto delay = [&](double rate) { return std::llround(position * rate - window(rate) / 2.0); };
	double rate = l.rates.first();
	for(double r : l.rates) {
		const long long d = delay(r);
		const int lo = l.triggerInBuffer ? std::max(l.delayMin, -window(r)) : l.delayMin;
		if(span * r <= l.timeBaseSamples && d >= lo && d <= l.delayMax) {
			rate = r;
		}
	}
	Sweep s;
	s.sampleRate = rate;
	s.bufferSize = window(rate);
	s.plotWindow = s.bufferSize;
	s.delay = static_cast<int>(
		std::clamp(delay(rate), static_cast<long long>(l.delayMin), static_cast<long long>(l.delayMax)));
	return normalized(s, l);
}

} // namespace scopy::adalm2000::sweep
