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

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "scopy-dcdebugger_export.h"

#include <QStringList>
#include <QVector>

namespace scopy::dcdebugger {

enum class Waveform
{
	Sine,
	Square,
	Triangle,
	Sawtooth,
	Constant
};

SCOPY_DCDEBUGGER_EXPORT QStringList waveformNames(); // in Waveform order

// One channel of the generator. Amplitude and offset are in full-scale units (1 = full scale).
struct ToneSpec
{
	bool enabled = true;
	Waveform waveform = Waveform::Sine;
	double frequency = 1000; // Hz
	double amplitude = 0.5;
	double offset = 0;
	double phase = 0; // degrees
};

// The frequency closest to `frequency` with a whole number of periods in `samples` at `fs`, so a
// replayed buffer has no seam (at least one period when frequency > 0).
SCOPY_DCDEBUGGER_EXPORT double snapFrequency(double frequency, double fs, int samples);

// `samples` values of the tone in full-scale units, clamped to [-1, 1]. The frequency is used as
// given (snap it first for a seamless replay).
SCOPY_DCDEBUGGER_EXPORT QVector<double> generateTone(const ToneSpec &tone, int samples, double fs);

} // namespace scopy::dcdebugger

#endif // WAVEFORM_H
