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

#include "waveform.h"

#include <algorithm>
#include <cmath>

using namespace scopy::dcdebugger;

QStringList scopy::dcdebugger::waveformNames() { return {"Sine", "Square", "Triangle", "Sawtooth", "Constant"}; }

double scopy::dcdebugger::snapFrequency(double frequency, double fs, int samples)
{
	if(fs <= 0 || samples <= 0 || frequency <= 0) {
		return std::max(0.0, frequency);
	}
	// Below Nyquist: at fs / 2 the tone has 2 samples per period and depends on its phase (a sine
	// at phase 0 samples as all zeros).
	const double maxCycles = std::floor((samples - 1) / 2.0);
	const double cycles = std::clamp(std::round(frequency * samples / fs), 1.0, std::max(1.0, maxCycles));
	return cycles * fs / samples;
}

QVector<double> scopy::dcdebugger::generateTone(const ToneSpec &tone, int samples, double fs)
{
	QVector<double> out(std::max(0, samples));
	const double cyclesPerSample = fs > 0 ? tone.frequency / fs : 0;
	const double phase0 = tone.phase / 360.0;
	for(int i = 0; i < out.size(); ++i) {
		double t = phase0 + cyclesPerSample * i; // in periods
		t -= std::floor(t);			 // [0, 1)
		double v = 0;
		switch(tone.waveform) {
		case Waveform::Sine:
			v = std::sin(2 * M_PI * t);
			break;
		case Waveform::Square:
			v = t < 0.5 ? 1.0 : -1.0;
			break;
		case Waveform::Triangle: // 0 at t = 0, rising like the sine
			v = t < 0.25 ? 4 * t : (t < 0.75 ? 2 - 4 * t : 4 * t - 4);
			break;
		case Waveform::Sawtooth:
			v = t < 0.5 ? 2 * t : 2 * t - 2;
			break;
		case Waveform::Constant:
			v = 0;
			break;
		}
		out[i] = std::clamp(tone.offset + tone.amplitude * v, -1.0, 1.0);
	}
	return out;
}
