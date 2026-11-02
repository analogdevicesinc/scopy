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

#include "signalmath.h"

#include "streamacquisition.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

using namespace scopy::dcdebugger;

namespace {
void fftInPlace(std::vector<std::complex<double>> &a)
{
	const size_t n = a.size();
	for(size_t i = 1, j = 0; i < n; ++i) {
		size_t bit = n >> 1;
		for(; j & bit; bit >>= 1) {
			j ^= bit;
		}
		j ^= bit;
		if(i < j) {
			std::swap(a[i], a[j]);
		}
	}
	for(size_t len = 2; len <= n; len <<= 1) {
		const double ang = -2 * M_PI / double(len);
		const std::complex<double> wlen(std::cos(ang), std::sin(ang));
		for(size_t i = 0; i < n; i += len) {
			std::complex<double> w(1);
			for(size_t k = 0; k < len / 2; ++k) {
				const std::complex<double> u = a[i + k];
				const std::complex<double> v = a[i + k + len / 2] * w;
				a[i + k] = u + v;
				a[i + k + len / 2] = u - v;
				w *= wlen;
			}
		}
	}
}
} // namespace

QStringList scopy::dcdebugger::windowNames()
{
	return {"Rectangular", "Hann", "Hamming", "Blackman-Harris", "Flat top"};
}

QVector<double> scopy::dcdebugger::makeWindow(WindowType type, int n)
{
	QVector<double> w(n, 1.0);
	if(n < 2) {
		return w;
	}
	// Periodic (DFT-even) windows: no leakage of exact bins, unlike the symmetric form.
	for(int i = 0; i < n; ++i) {
		const double x = 2 * M_PI * i / double(n);
		switch(type) {
		case WindowType::Rectangular:
			break;
		case WindowType::Hann:
			w[i] = 0.5 - 0.5 * std::cos(x);
			break;
		case WindowType::Hamming:
			w[i] = 0.54 - 0.46 * std::cos(x);
			break;
		case WindowType::BlackmanHarris:
			w[i] = 0.35875 - 0.48829 * std::cos(x) + 0.14128 * std::cos(2 * x) - 0.01168 * std::cos(3 * x);
			break;
		case WindowType::FlatTop:
			w[i] = 0.21557895 - 0.41663158 * std::cos(x) + 0.277263158 * std::cos(2 * x) -
				0.083578947 * std::cos(3 * x) + 0.006947368 * std::cos(4 * x);
			break;
		}
	}
	return w;
}

int scopy::dcdebugger::floorPow2(int n)
{
	if(n < 1) {
		return 0;
	}
	int p = 1;
	while(p <= n / 2) {
		p <<= 1;
	}
	return p;
}

QVector<double> scopy::dcdebugger::amplitudeSpectrum(const QVector<double> &x, WindowType window)
{
	const int n = floorPow2(int(x.size()));
	if(n < 2) {
		return {};
	}
	const QVector<double> w = makeWindow(window, n);
	double gain = 0;
	std::vector<std::complex<double>> a(static_cast<size_t>(n));
	for(int i = 0; i < n; ++i) {
		a[size_t(i)] = x[i] * w[i];
		gain += w[i];
	}
	fftInPlace(a);
	QVector<double> out(n / 2 + 1);
	for(int k = 0; k <= n / 2; ++k) {
		const double mag = std::abs(a[size_t(k)]) / gain;
		out[k] = (k == 0 || k == n / 2) ? mag : 2 * mag;
	}
	return out;
}

QVector<double> scopy::dcdebugger::complexAmplitudeSpectrum(const QVector<double> &i, const QVector<double> &q,
							    WindowType window)
{
	const int len = q.isEmpty() ? int(i.size()) : std::min(int(i.size()), int(q.size()));
	const int n = floorPow2(len);
	if(n < 2) {
		return {};
	}
	const QVector<double> w = makeWindow(window, n);
	double gain = 0;
	std::vector<std::complex<double>> a(static_cast<size_t>(n));
	for(int s = 0; s < n; ++s) {
		a[size_t(s)] = std::complex<double>(i[s], q.isEmpty() ? 0.0 : q[s]) * w[s];
		gain += w[s];
	}
	fftInPlace(a);
	QVector<double> out(n);
	for(int k = 0; k < n; ++k) {
		// fftshift: negative frequencies first.
		out[k] = std::abs(a[size_t((k + n / 2) % n)]) / gain;
	}
	return out;
}

double scopy::dcdebugger::toDb(double v, double ref)
{
	if(v <= 0 || ref <= 0) {
		return -300;
	}
	return std::max(-300.0, 20 * std::log10(v / ref));
}

void SpectrumAverager::setCount(int count)
{
	count = std::max(1, count);
	if(count != m_count) {
		m_count = count;
		reset();
	}
}

void SpectrumAverager::reset() { m_history.clear(); }

QVector<double> SpectrumAverager::push(const QString &key, const QVector<double> &amplitude)
{
	if(m_count <= 1) {
		return amplitude;
	}
	History &h = m_history[key];
	if(!h.sum.isEmpty() && h.sum.size() != amplitude.size()) {
		h = History{}; // size changed: start over
	}
	if(h.sum.isEmpty()) {
		h.sum.fill(0, amplitude.size());
	}
	QVector<double> power(amplitude.size());
	for(int i = 0; i < amplitude.size(); ++i) {
		power[i] = amplitude[i] * amplitude[i];
		h.sum[i] += power[i];
	}
	h.power.append(power);
	while(h.power.size() > m_count) {
		const QVector<double> &old = h.power.first();
		for(int i = 0; i < old.size(); ++i) {
			h.sum[i] -= old[i];
		}
		h.power.removeFirst();
	}
	QVector<double> out(amplitude.size());
	const double n = double(h.power.size());
	for(int i = 0; i < out.size(); ++i) {
		out[i] = std::sqrt(std::max(0.0, h.sum[i] / n));
	}
	return out;
}

Measurements scopy::dcdebugger::measure(const QVector<double> &data, int from, int count, double fs)
{
	Measurements m;
	from = std::clamp(from, 0, int(data.size()));
	count = std::clamp(count, 0, int(data.size()) - from);
	if(count == 0) {
		return m;
	}
	m.min = m.max = data[from];
	double sum = 0;
	double sumSq = 0;
	for(int i = from; i < from + count; ++i) {
		const double v = data[i];
		m.min = std::min(m.min, v);
		m.max = std::max(m.max, v);
		sum += v;
		sumSq += v * v;
	}
	m.mean = sum / count;
	m.rms = std::sqrt(sumSq / count);
	m.acRms = std::sqrt(std::max(0.0, sumSq / count - m.mean * m.mean));
	m.pkpk = m.max - m.min;

	// Rising crossings of the mean (10% hysteresis); frequency from the first to the last one.
	if(fs > 0 && m.pkpk > 0) {
		const QVector<double> window = data.mid(from, count);
		const double hysteresis = m.pkpk * 0.1;
		int first = -1;
		int last = -1;
		int periods = -1;
		for(int at = findEdge(window, m.mean, true, 0, -1, hysteresis); at >= 0;
		    at = findEdge(window, m.mean, true, at + 1, -1, hysteresis)) {
			if(first < 0) {
				first = at;
			}
			last = at;
			periods++;
		}
		if(periods >= 1 && last > first) {
			m.frequency = periods * fs / double(last - first);
		}
	}
	return m;
}
