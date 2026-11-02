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

#include "acquisition/signalmath.h"

#include <QtTest>

#include <cmath>

using namespace scopy::dcdebugger;

namespace {
QVector<double> sine(int n, double amplitude, double cyclesPerBuffer, double offset = 0)
{
	QVector<double> out(n);
	for(int i = 0; i < n; ++i) {
		out[i] = offset + amplitude * std::sin(2 * M_PI * cyclesPerBuffer * i / n);
	}
	return out;
}
} // namespace

class TST_SignalMath : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void floorPow2Values();
	void windowsAreNormalized();
	void sineAmplitudeLandsInItsBin_data();
	void sineAmplitudeLandsInItsBin();
	void dcLandsInBinZero();
	void complexToneHasOneSide();
	void decibels();
	void averagerKeepsLastN();
	void measureSine();
	void measureWithoutSampleRate();
};

void TST_SignalMath::floorPow2Values()
{
	QCOMPARE(floorPow2(0), 0);
	QCOMPARE(floorPow2(1), 1);
	QCOMPARE(floorPow2(4096), 4096);
	QCOMPARE(floorPow2(5000), 4096);
}

void TST_SignalMath::windowsAreNormalized()
{
	QCOMPARE(windowNames().size(), 5);
	const QVector<double> hann = makeWindow(WindowType::Hann, 8); // periodic: peak at n / 2
	QCOMPARE(hann.first(), 0.0);
	QCOMPARE(hann[4], 1.0);
	QCOMPARE(makeWindow(WindowType::Rectangular, 4), QVector<double>({1, 1, 1, 1}));
}

void TST_SignalMath::sineAmplitudeLandsInItsBin_data()
{
	QTest::addColumn<int>("window");
	for(int w = 0; w < windowNames().size(); ++w) {
		QTest::newRow(qPrintable(windowNames()[w])) << w;
	}
}

void TST_SignalMath::sineAmplitudeLandsInItsBin()
{
	QFETCH(int, window);
	const QVector<double> amp = amplitudeSpectrum(sine(1024, 3.0, 64), WindowType(window));
	QCOMPARE(amp.size(), 513);
	const int peak = int(std::max_element(amp.cbegin(), amp.cend()) - amp.cbegin());
	QCOMPARE(peak, 64);
	// Coherent sine: every window reads the amplitude in its bin.
	QVERIFY(std::abs(amp[64] - 3.0) < 1e-6);
}

void TST_SignalMath::dcLandsInBinZero()
{
	const QVector<double> amp = amplitudeSpectrum(QVector<double>(256, 2.5), WindowType::Hann);
	QVERIFY(std::abs(amp[0] - 2.5) < 1e-9);
	QVERIFY(amp[10] < 1e-9);
}

void TST_SignalMath::complexToneHasOneSide()
{
	// i + jq = A e^{+j w t}: one line at +f, none at -f.
	const int n = 512;
	QVector<double> i(n);
	QVector<double> q(n);
	for(int s = 0; s < n; ++s) {
		i[s] = 2.0 * std::cos(2 * M_PI * 32 * s / n);
		q[s] = 2.0 * std::sin(2 * M_PI * 32 * s / n);
	}
	const QVector<double> amp = complexAmplitudeSpectrum(i, q, WindowType::Hann);
	QCOMPARE(amp.size(), n);
	const int center = n / 2;
	QVERIFY(std::abs(amp[center + 32] - 2.0) < 1e-6);
	QVERIFY(amp[center - 32] < 1e-6);
	// Real input: the line splits in half on both sides.
	const QVector<double> real = complexAmplitudeSpectrum(i, {}, WindowType::Hann);
	QVERIFY(std::abs(real[center + 32] - 1.0) < 1e-6);
	QVERIFY(std::abs(real[center - 32] - 1.0) < 1e-6);
}

void TST_SignalMath::decibels()
{
	QCOMPARE(toDb(1.0), 0.0);
	QVERIFY(std::abs(toDb(1024, 2048) + 6.0206) < 1e-3);
	QCOMPARE(toDb(0), -300.0);
}

void TST_SignalMath::averagerKeepsLastN()
{
	SpectrumAverager avg;
	QCOMPARE(avg.push("a", {2}), QVector<double>({2})); // count 1: pass-through
	avg.setCount(2);
	QCOMPARE(avg.push("a", {3}), QVector<double>({3}));
	const QVector<double> two = avg.push("a", {4});
	QVERIFY(std::abs(two[0] - std::sqrt((9.0 + 16.0) / 2)) < 1e-12);
	const QVector<double> slid = avg.push("a", {0});
	QVERIFY(std::abs(slid[0] - std::sqrt(16.0 / 2)) < 1e-12); // 3 dropped
	QCOMPARE(avg.push("b", {5}), QVector<double>({5}));	  // keys are independent
}

void TST_SignalMath::measureSine()
{
	// 10 periods of a 1 kHz sine at 100 kHz, amplitude 2 around 1.
	const QVector<double> d = sine(1000, 2.0, 10, 1.0);
	const Measurements m = measure(d, 0, d.size(), 100e3);
	QVERIFY(std::abs(m.max - 3.0) < 1e-3);
	QVERIFY(std::abs(m.min + 1.0) < 1e-3);
	QVERIFY(std::abs(m.mean - 1.0) < 1e-9);
	QVERIFY(std::abs(m.pkpk - 4.0) < 2e-3);
	QVERIFY(std::abs(m.acRms - 2.0 / std::sqrt(2.0)) < 1e-6);
	QVERIFY(std::abs(m.rms - std::sqrt(1.0 + 2.0)) < 1e-6);
	QVERIFY(std::abs(m.frequency - 1000.0) < 1.0);
}

void TST_SignalMath::measureWithoutSampleRate()
{
	const Measurements m = measure(sine(100, 1, 5), 0, 100, 0);
	QVERIFY(m.frequency < 0);
	QCOMPARE(measure({}, 0, 10, 1).pkpk, 0.0);
}

QTEST_GUILESS_MAIN(TST_SignalMath)
#include "tst_signalmath.moc"
