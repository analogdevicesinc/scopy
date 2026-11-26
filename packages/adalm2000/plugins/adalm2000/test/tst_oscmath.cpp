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

#include "oscmath.h"

#include <QTest>

using namespace scopy::adalm2000::osc;

class TST_OscMath : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void listedRatesNeedNoOversampling()
	{
		for(double r : DEFAULT_HARDWARE_RATES) {
			const HwRate hw = hwRate(r, {});
			QCOMPARE(hw.rate, r);
			QCOMPARE(hw.oversampling, 1);
		}
	}

	void otherRatesDivideTheNextOneUp()
	{
		QCOMPARE(hwRate(2e6, {}).rate, 1e7);
		QCOMPARE(hwRate(2e6, {}).oversampling, 5);
		QCOMPARE(hwRate(5e3, {}).oversampling, 2);
		QCOMPARE(hwRate(3e6, {}).oversampling, 3);
	}

	void outOfRangeClamps()
	{
		QCOMPARE(hwRate(1e9, {}).rate, 100e6);
		QCOMPARE(hwRate(1e9, {}).oversampling, 1);
		QCOMPARE(hwRate(10.0, {}).rate, 1e3);
	}

	void unsortedListsWork() { QCOMPARE(hwRate(2e5, {1e8, 1e6, 1e3}).rate, 1e6); }

	void ceilTo125()
	{
		QCOMPARE(ceil125(1.0), 1.0);
		QCOMPARE(ceil125(1.1), 2.0);
		QCOMPARE(ceil125(3e-3), 5e-3);
		QCOMPARE(ceil125(6.0), 10.0);
		QCOMPARE(ceil125(0.0), 0.0);
	}

	void autosetShowsAFewPeriods()
	{
		// 1 kHz: 2.5 ms over 10 divisions is 250 us/div, up to 500 us.
		QCOMPARE(autosetTimeBase(1e-3, 10), 500e-6);
		QCOMPARE(autosetTimeBase(-1, 10), AUTOSET_DEFAULT_TIME_BASE);
	}

	void autosetFitsTheAmplitude()
	{
		QCOMPARE(autosetVoltsPerDiv(4.0), 0.5);
		QCOMPARE(autosetVoltsPerDiv(0.0), AUTOSET_MIN_VOLTS_PER_DIV);
	}
};

QTEST_MAIN(TST_OscMath)
#include "tst_oscmath.moc"
