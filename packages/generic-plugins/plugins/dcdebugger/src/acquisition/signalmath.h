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

#ifndef SIGNALMATH_H
#define SIGNALMATH_H

#include "scopy-dcdebugger_export.h"

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

namespace scopy::dcdebugger {

enum class WindowType
{
	Rectangular,
	Hann,
	Hamming,
	BlackmanHarris,
	FlatTop
};

SCOPY_DCDEBUGGER_EXPORT QStringList windowNames(); // in WindowType order
SCOPY_DCDEBUGGER_EXPORT QVector<double> makeWindow(WindowType type, int n);

// Largest power of two <= n (0 for n < 1).
SCOPY_DCDEBUGGER_EXPORT int floorPow2(int n);

// Single-sided amplitude spectrum of the first floorPow2(size) samples: bin k holds the peak
// amplitude of the component at k * fs / N, window gain compensated (a sine of amplitude A reads
// A in its bin). N / 2 + 1 bins.
SCOPY_DCDEBUGGER_EXPORT QVector<double> amplitudeSpectrum(const QVector<double> &x, WindowType window);

// Two-sided amplitude spectrum of the complex signal i + jq (first floorPow2 samples), shifted so
// bin k is the component at (k - N / 2) * fs / N; a complex exponential of amplitude A reads A.
// q may be empty (real input). N bins.
SCOPY_DCDEBUGGER_EXPORT QVector<double> complexAmplitudeSpectrum(const QVector<double> &i, const QVector<double> &q,
								 WindowType window);

// 20 * log10(v / ref), floored at -300 dB.
SCOPY_DCDEBUGGER_EXPORT double toDb(double v, double ref = 1.0);

// Power average of the last `count` spectra of each key (count 1 = no averaging).
class SCOPY_DCDEBUGGER_EXPORT SpectrumAverager
{
public:
	void setCount(int count);
	int count() const { return m_count; }
	void reset();
	// Adds one amplitude spectrum and returns the averaged amplitude spectrum.
	QVector<double> push(const QString &key, const QVector<double> &amplitude);

private:
	struct History
	{
		QList<QVector<double>> power;
		QVector<double> sum;
	};
	int m_count = 1;
	QHash<QString, History> m_history;
};

struct Measurements
{
	double min = 0;
	double max = 0;
	double mean = 0;
	double rms = 0;
	double acRms = 0; // RMS around the mean
	double pkpk = 0;
	double frequency = -1; // Hz; < 0 when unknown (no sample rate or under two periods)
};

// Statistics of data[from, from + count) sampled at fs (Hz, 0 = unknown).
SCOPY_DCDEBUGGER_EXPORT Measurements measure(const QVector<double> &data, int from, int count, double fs);

} // namespace scopy::dcdebugger

#endif // SIGNALMATH_H
