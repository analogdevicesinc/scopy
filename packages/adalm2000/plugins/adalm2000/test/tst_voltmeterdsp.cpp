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

#include "voltmeterdsp.h"

#include <QTest>
#include <cmath>

using namespace scopy::adalm2000::dsp;

class TST_VoltmeterDsp : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void movingAverageOfConstantIsThatConstant();
	void dcBlockerRemovesConstant();
	void rmsOfSquareWaveIsAmplitude();
	void updateArrivesEveryDecimationSamples();
	void autoGainNeedsAllTwentyFiveToDropDown();
	void autoGainGoesUpOnOneFrame();
};

void TST_VoltmeterDsp::movingAverageOfConstantIsThatConstant()
{
	MovingAverage ma(4);
	for(int i = 0; i < 8; ++i) {
		ma.push(7.0f);
	}
	QVERIFY(ma.warm());
	QVERIFY(std::abs(ma.push(7.0f) - 7.0f) < 1e-4f);
}

void TST_VoltmeterDsp::dcBlockerRemovesConstant()
{
	DcBlocker b(16);
	float out = 0.0f;
	for(int i = 0; i < 400; ++i) {
		out = b.highpass(5.0f);
	}
	QVERIFY(std::abs(out) < 1e-3f);
}

void TST_VoltmeterDsp::rmsOfSquareWaveIsAmplitude()
{
	RmsIir r(0.05);
	float out = 0.0f;
	for(int i = 0; i < 4000; ++i) {
		out = r.push((i % 2) ? 3.0f : -3.0f);
	}
	QVERIFY(std::abs(out - 3.0f) < 0.05f);
}

void TST_VoltmeterDsp::updateArrivesEveryDecimationSamples()
{
	VoltmeterChannel ch;
	int updates = 0;
	for(std::size_t i = 0; i < DECIMATION * 3; ++i) {
		if(ch.push(100)) {
			++updates;
		}
	}
	QCOMPARE(updates, 3);
}

void TST_VoltmeterDsp::autoGainNeedsAllTwentyFiveToDropDown()
{
	AutoGain g;
	for(int i = 0; i < 24; ++i) {
		QCOMPARE(g.push(-0.5, 0.5), AutoGain::Range::Plus25V);
	}
	QCOMPARE(g.push(-0.5, 0.5), AutoGain::Range::Plus2_5V);
}

void TST_VoltmeterDsp::autoGainGoesUpOnOneFrame()
{
	AutoGain g;
	for(int i = 0; i < 25; ++i) {
		g.push(-0.5, 0.5);
	}
	QCOMPARE(g.push(-10.0, 10.0), AutoGain::Range::Plus25V);
}

QTEST_MAIN(TST_VoltmeterDsp)

#include "tst_voltmeterdsp.moc"
