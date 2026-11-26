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

#include "sweepmath.h"

#include <QTest>

using namespace scopy::adalm2000::sweep;

class TST_SweepMath : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void seriesIsOneTwoFive();
	void timeBaseRoundTrips();
	void timeBasePicksTheFastestRateThatFits();
	void longSpansClampAtTheSlowestRate();
	void pretriggerLowersTheRate();
	void countsAreMultiplesOfFour();
	void windowEditsKeepTheTriggerPlace();
	void normalizedKeepsTheWindowOverTheBuffer();
	void slowTimeBasesStream();

private:
	static Limits limits();
};

Limits TST_SweepMath::limits()
{
	Limits l;
	l.rates = series125(1.0, 100e6);
	l.minSamples = 100;
	l.maxBuffer = 1 << 20;
	l.maxWindow = 1 << 22;
	l.timeBaseSamples = 1 << 16;
	l.delayMin = -8192;
	l.delayMax = 8191;
	return l;
}

void TST_SweepMath::seriesIsOneTwoFive()
{
	QCOMPARE(series125(1e3, 1e4), (QList<double>{1e3, 2e3, 5e3, 1e4}));
	QCOMPARE(series125(1.0, 100e6).last(), 100e6);
}

void TST_SweepMath::timeBaseRoundTrips()
{
	const Sweep s = fromTimeBase(1e-3, 0.0, 10, limits());
	QCOMPARE(s.timeBase(10), 1e-3);
	QCOMPARE(s.position(), 0.0);
	QCOMPARE(s.plotWindow, s.bufferSize);
	QCOMPARE(s.delay, -s.plotWindow / 2);
}

void TST_SweepMath::timeBasePicksTheFastestRateThatFits()
{
	// 10 ms span from the trigger: 5 MHz gives 50000 samples, 10 MHz would need 100000.
	const Sweep s = fromTimeBase(1e-3, 5e-3, 10, limits());
	QCOMPARE(s.sampleRate, 5e6);
	QCOMPARE(s.plotWindow, 50000);
	QCOMPARE(s.delay, 0);
	// Centered, 25000 samples before the trigger would not fit the 8192 the hardware keeps.
	QCOMPARE(fromTimeBase(1e-3, 0.0, 10, limits()).sampleRate, 1e6);
	QCOMPARE(fromTimeBase(100e-9, 0.0, 10, limits()).sampleRate, 100e6);
}

void TST_SweepMath::longSpansClampAtTheSlowestRate()
{
	const Sweep s = fromTimeBase(1e6, 0.0, 10, limits());
	QCOMPARE(s.sampleRate, 1.0);
	QCOMPARE(s.plotWindow, 1 << 16);
}

void TST_SweepMath::pretriggerLowersTheRate()
{
	// 1 ms before the trigger is 100000 samples at 100 MHz, past the 8192 the hardware keeps.
	const Sweep s = fromTimeBase(10e-6, -1e-3, 10, limits());
	QCOMPARE(s.sampleRate, 5e6);
	QVERIFY(s.delay >= -8192);
	QVERIFY(qAbs(s.position() + 1e-3) <= 2.5 / s.sampleRate);
}

void TST_SweepMath::countsAreMultiplesOfFour()
{
	Sweep s;
	s.bufferSize = 101;
	s.plotWindow = 1003;
	s.delay = -7;
	s = normalized(s, limits());
	QCOMPARE(s.bufferSize, 100);
	QCOMPARE(s.plotWindow, 1004);
	QCOMPARE(s.delay, -8);
	QCOMPARE(align4(8191, -8192, 8191), 8188);
	QCOMPARE(align4(50, 100, 1000), 100);
}

void TST_SweepMath::windowEditsKeepTheTriggerPlace()
{
	Sweep s;
	s.bufferSize = 1000;
	s.plotWindow = 1000;
	s.delay = -500;
	const Sweep w = withWindow(s, 2000, 2000, limits());
	QCOMPARE(w.delay, -1000);
	s.delay = 0;
	QCOMPARE(withWindow(s, 4000, 4000, limits()).delay, 0);
}

void TST_SweepMath::normalizedKeepsTheWindowOverTheBuffer()
{
	Sweep s;
	s.bufferSize = 5000;
	s.plotWindow = 1000;
	s.sampleRate = 1e9;
	s.delay = -1e6;
	const Sweep n = normalized(s, limits());
	QCOMPARE(n.plotWindow, 5000);
	QCOMPARE(n.sampleRate, 100e6);
	QCOMPARE(n.delay, -8192);
	Limits inBuffer = limits();
	inBuffer.triggerInBuffer = true;
	QCOMPARE(normalized(s, inBuffer).delay, -5000);
}

void TST_SweepMath::slowTimeBasesStream()
{
	Limits l = limits();
	l.timeBaseSamples = 8192;
	l.maxWindow = 1 << 21;
	l.streamTimeBase = 0.1;
	const Sweep fast = fromTimeBase(10e-3, 0.0, 10, l);
	QCOMPARE(fast.plotWindow, fast.bufferSize);
	const Sweep slow = fromTimeBase(1.0, 0.0, 10, l);
	QVERIFY(slow.plotWindow > slow.bufferSize);
	QCOMPARE(slow.delay, 0);
	QCOMPARE(slow.bufferSize, slow.plotWindow / STREAM_BUFFERS_PER_WINDOW);
	QVERIFY(qAbs(slow.timeBase(10) - 1.0) < 1e-9);
}

QTEST_MAIN(TST_SweepMath)

#include "tst_sweepmath.moc"
