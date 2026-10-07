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

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <optional>
#include <vector>

namespace scopy::adalm2000::dsp {

// At sample_rate = 1e5.
inline constexpr std::size_t DECIMATION = 10000; // keep_one_in_n -> 10 updates/s
inline constexpr std::size_t BLOCKER_LEN = 4000; // dc_blocker_ff, long form
inline constexpr std::size_t MA_LEN = 4000;	 // moving_average_ff, gain 1/4000
inline constexpr double RMS_ALPHA = 1e-4;	 // rms_ff single-pole IIR
inline constexpr std::size_t WINDOW = 10000;	 // min_ff / max_ff window

// gr::blocks::moving_average_ff with length N and gain 1/N: a true mean.
class MovingAverage
{
public:
	explicit MovingAverage(std::size_t length)
		: m_buf(length, 0.0f)
	{}

	float push(float x)
	{
		m_sum += static_cast<double>(x) - static_cast<double>(m_buf[m_pos]);
		m_buf[m_pos] = x;
		m_pos = (m_pos + 1) % m_buf.size();
		if(m_filled < m_buf.size()) {
			++m_filled;
		}
		return static_cast<float>(m_sum / static_cast<double>(m_buf.size()));
	}

	bool warm() const { return m_filled >= m_buf.size(); }

	void reset()
	{
		std::fill(m_buf.begin(), m_buf.end(), 0.0f);
		m_sum = 0.0;
		m_pos = 0;
		m_filled = 0;
	}

private:
	std::vector<float> m_buf;
	std::size_t m_pos = 0;
	std::size_t m_filled = 0;
	double m_sum = 0.0;
};

// gr::filter::dc_blocker_ff, long form: the input delayed by D-1, minus two
// cascaded D-tap moving averages. Group delay 2D-2 = 7998 at D = 4000.
class DcBlocker
{
public:
	explicit DcBlocker(std::size_t d)
		: m_ma0(d)
		, m_ma1(d)
		, m_delay(d - 1, 0.0f)
	{}

	float highpass(float x)
	{
		// The subtraction is sample-wise, not delay-aligned: this reproduces GNU Radio's
		// dc_blocker_ff topology. Aligning the two paths turns it into a plain low-pass.
		const float delayed = m_delay[m_pos];
		m_delay[m_pos] = x;
		m_pos = (m_pos + 1) % m_delay.size();
		return delayed - m_ma1.push(m_ma0.push(x));
	}

	void reset()
	{
		m_ma0.reset();
		m_ma1.reset();
		std::fill(m_delay.begin(), m_delay.end(), 0.0f);
		m_pos = 0;
	}

private:
	MovingAverage m_ma0;
	MovingAverage m_ma1;
	std::vector<float> m_delay;
	std::size_t m_pos = 0;
};

// gr::blocks::rms_ff(alpha): a single-pole IIR on the squared signal, not a block
// RMS. alpha = 1e-4 is roughly a 100 ms time constant at 1e5 samples/s.
class RmsIir
{
public:
	explicit RmsIir(double alpha)
		: m_alpha(alpha)
	{}

	float push(float x)
	{
		const double sq = static_cast<double>(x) * static_cast<double>(x);
		m_avg = m_avg * (1.0 - m_alpha) + m_alpha * sq;
		return static_cast<float>(std::sqrt(m_avg));
	}

	void reset() { m_avg = 0.0; }

private:
	double m_alpha;
	double m_avg = 0.0;
};

// One decimated update, in raw ADC counts; the caller converts with
// M2kAnalogIn::convertRawToVolts.
struct Update
{
	int dcCount = 0;
	int acCount = 0;
	short blockMin = 0;
	short blockMax = 0;
};

// Feed raw ADC counts; an Update comes back on every DECIMATION-th sample.
class VoltmeterChannel
{
public:
	VoltmeterChannel()
		: m_dcBlocker(BLOCKER_LEN)
		, m_acBlocker(BLOCKER_LEN)
		, m_ma(MA_LEN)
		, m_rms(RMS_ALPHA)
	{}

	std::optional<Update> push(short raw)
	{
		const float x = static_cast<float>(raw);

		m_lastDc = m_ma.push(x - m_dcBlocker.highpass(x));
		m_lastAc = m_rms.push(m_acBlocker.highpass(x));

		if(m_inWindow == 0) {
			m_min = raw;
			m_max = raw;
		} else {
			m_min = std::min(m_min, raw);
			m_max = std::max(m_max, raw);
		}
		++m_inWindow;

		if(m_inWindow < DECIMATION) {
			return std::nullopt;
		}
		m_inWindow = 0;

		// One LSB is about 14.5 mV in the +/-25 V range.
		Update u;
		u.dcCount = static_cast<int>(m_lastDc);
		u.acCount = static_cast<int>(m_lastAc);
		u.blockMin = m_min;
		u.blockMax = m_max;
		return u;
	}

	void reset()
	{
		m_dcBlocker.reset();
		m_acBlocker.reset();
		m_ma.reset();
		m_rms.reset();
		m_inWindow = 0;
		m_min = 0;
		m_max = 0;
		m_lastDc = 0.0f;
		m_lastAc = 0.0f;
	}

private:
	DcBlocker m_dcBlocker;
	DcBlocker m_acBlocker;
	MovingAverage m_ma;
	RmsIir m_rms;
	std::size_t m_inWindow = 0;
	short m_min = 0;
	short m_max = 0;
	float m_lastDc = 0.0f;
	float m_lastAc = 0.0f;
};

// Drops to Plus2_5V only when all DEPTH history entries agree; returns to
// Plus25V on a single out-of-range frame.
class AutoGain
{
public:
	enum class Range
	{
		Plus25V,
		Plus2_5V
	};

	static constexpr std::size_t DEPTH = 25;
	static constexpr double LOW_RANGE_LIMIT = 2.5;

	AutoGain()
		: m_hist(DEPTH, Range::Plus25V)
	{}

	Range push(double blockMinV, double blockMaxV)
	{
		const bool fitsLow = std::max(std::abs(blockMinV), std::abs(blockMaxV)) < LOW_RANGE_LIMIT;
		m_hist.push_back(fitsLow ? Range::Plus2_5V : Range::Plus25V);
		m_hist.pop_front();

		for(Range r : m_hist) {
			if(r != Range::Plus2_5V) {
				return Range::Plus25V;
			}
		}
		return Range::Plus2_5V;
	}

	void reset() { m_hist.assign(DEPTH, Range::Plus25V); }

private:
	std::deque<Range> m_hist;
};

} // namespace scopy::adalm2000::dsp
