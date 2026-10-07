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

#include "powersupplymath.h"

#include <QTest>
#include <cmath>

using namespace scopy::adalm2000::ps;

class TST_PowerSupplyMath : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void averageDividesByHeldCountNotWindowSize();
	void averageWindowStopsAtFiveSamples();
	void clearedAverageSnapsToTheNextSample();
	void trackingMatchesTheDocumentedExample();
	void trackingAtFullRatioMirrorsThePositiveRail();
	void trackingAtZeroRatioIsZero();
};

void TST_PowerSupplyMath::averageDividesByHeldCountNotWindowSize()
{
	RailAverage a;
	QVERIFY(std::abs(a.push(3.0) - 3.0) < 1e-12);
	QCOMPARE(a.size(), std::size_t(1));
	QVERIFY(std::abs(a.push(1.0) - 2.0) < 1e-12);
}

void TST_PowerSupplyMath::averageWindowStopsAtFiveSamples()
{
	RailAverage a;
	for(int i = 0; i < 5; ++i) {
		a.push(0.0);
	}
	QCOMPARE(a.size(), AVERAGE_COUNT);
	a.push(5.0);
	QCOMPARE(a.size(), AVERAGE_COUNT);
	QVERIFY(std::abs(a.value() - 1.0) < 1e-12);
}

void TST_PowerSupplyMath::clearedAverageSnapsToTheNextSample()
{
	RailAverage a;
	for(int i = 0; i < 5; ++i) {
		a.push(1.0);
	}
	QVERIFY(std::abs(a.value() - 1.0) < 1e-12);

	a.clear();
	QCOMPARE(a.size(), std::size_t(0));
	// Without the clear this would read (1+1+1+1+4)/5 = 1.6.
	QVERIFY(std::abs(a.push(4.0) - 4.0) < 1e-12);
}

// 1.0 V at 70 % gives -0.7 V.
void TST_PowerSupplyMath::trackingMatchesTheDocumentedExample()
{
	QVERIFY(std::abs(trackingNegative(1.0, 70) - (-0.7)) < 1e-12);
}

void TST_PowerSupplyMath::trackingAtFullRatioMirrorsThePositiveRail()
{
	QVERIFY(std::abs(trackingNegative(5.0, RATIO_DEFAULT) - (-5.0)) < 1e-12);
	QCOMPARE(RATIO_DEFAULT, 100);
}

void TST_PowerSupplyMath::trackingAtZeroRatioIsZero() { QVERIFY(std::abs(trackingNegative(5.0, 0)) < 1e-12); }

QTEST_MAIN(TST_PowerSupplyMath)

#include "tst_powersupplymath.moc"
