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

#include "spectrumsweep.h"

#include <QTest>

using namespace scopy::adalm2000::spec;

class TST_SpectrumSweep : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void fullBandRunsAtTheMaximumRate()
	{
		QCOMPARE(oversamplingFor(MAX_STOP_HZ), 1);
		QCOMPARE(sampleRateFor(MAX_STOP_HZ), MAX_SAMPLE_RATE);
	}

	void rateStillCoversTheStopFrequency()
	{
		for(double stop : {1e3, 7.3e3, 1e6, 12.5e6, 33e6}) {
			QVERIFY2(sampleRateFor(stop) / 2.0 >= stop, qPrintable(QString::number(stop)));
		}
		// 1 MHz: 100 MSPS / 50 = 2 MSPS exactly.
		QCOMPARE(oversamplingFor(1e6), 50);
	}

	void stopIsClampedToTheBand()
	{
		QCOMPARE(oversamplingFor(1e9), 1);
		QCOMPARE(oversamplingFor(0.0), oversamplingFor(MIN_STOP_HZ));
	}

	void fftSizesArePowersOfTwo()
	{
		const QList<int> sizes = fftSizes();
		QCOMPARE(sizes.first(), 256);
		QCOMPARE(sizes.last(), 1 << 18);
		for(int n : sizes) {
			QVERIFY((n & (n - 1)) == 0);
		}
		QCOMPARE(bins(1024), 513);
		QCOMPARE(rbw(1e6, 1000), 1000.0);
	}

	void startKeepsStopUnlessTooNarrow()
	{
		const Span s = withStart({0, 10e6}, 2e6, 1e3);
		QCOMPARE(s.start, 2e6);
		QCOMPARE(s.stop, 10e6);

		// Past the stop: the stop is pushed out by the minimum span.
		const Span pushed = withStart({0, 10e6}, 11e6, 1e3);
		QCOMPARE(pushed.start, 11e6);
		QCOMPARE(pushed.stop, 11e6 + 1e3);
	}

	void stopKeepsStartUnlessTooNarrow()
	{
		const Span s = withStop({1e6, 10e6}, 5e6, 1e3);
		QCOMPARE(s.start, 1e6);
		QCOMPARE(s.stop, 5e6);

		const Span pulled = withStop({1e6, 10e6}, 0.5e6, 1e3);
		QCOMPARE(pulled.stop, 0.5e6);
		QCOMPARE(pulled.start, 0.5e6 - 1e3);
	}

	void centerKeepsWidthAndSlidesInsideTheBand()
	{
		const Span s = withCenter({0, 10e6}, 20e6, 1e3);
		QCOMPARE(s.width(), 10e6);
		QCOMPARE(s.center(), 20e6);

		const Span edge = withCenter({0, 10e6}, 1e6, 1e3);
		QCOMPARE(edge.start, 0.0);
		QCOMPARE(edge.stop, 10e6);

		const Span top = withCenter({0, 10e6}, MAX_STOP_HZ, 1e3);
		QCOMPARE(top.stop, MAX_STOP_HZ);
		QCOMPARE(top.width(), 10e6);
	}

	void widthKeepsCenterAndShiftsAtTheEdge()
	{
		const Span s = withWidth({10e6, 20e6}, 4e6, 1e3);
		QCOMPARE(s.center(), 15e6);
		QCOMPARE(s.width(), 4e6);

		const Span edge = withWidth({0, 2e6}, 10e6, 1e3);
		QCOMPARE(edge.start, 0.0);
		QCOMPARE(edge.width(), 10e6);

		const Span floor = withWidth({10e6, 20e6}, 1.0, 1e3);
		QCOMPARE(floor.width(), 1e3);
	}

	void minSpanIsTenBins() { QCOMPARE(minSpan(1e6, 1000), 10e3); }
};

QTEST_GUILESS_MAIN(TST_SpectrumSweep)
#include "tst_spectrumsweep.moc"
