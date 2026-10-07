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

#include "rangepresetscaler.h"

#include <QTest>

using namespace scopy;

class TST_RangePresetScaler : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void startsAtTheSmallestPreset();
	void pushWidensButNeverNarrows();
	void pushReturnsMinusOneWhenTheRangeStillFits();
	void settleDropsToTheTightestBracket();
	void settleReturnsMinusOneWhenAlreadyTightest();
	void floorAtZeroClampsTheLowerBound();
	void setPresetsReplacesTheTableAndResets();
};

void TST_RangePresetScaler::startsAtTheSmallestPreset()
{
	RangePresetScaler s;
	QCOMPARE(s.index(), 0);
	QCOMPARE(s.current().upper, 0.1);
	QCOMPARE(s.current().lower, -0.1);
}

void TST_RangePresetScaler::pushWidensButNeverNarrows()
{
	RangePresetScaler s;
	QCOMPARE(s.push(3.0), 2);
	QCOMPARE(s.index(), 2);

	// A small value must not pull the range back down; only settle() does that.
	QCOMPARE(s.push(0.01), -1);
	QCOMPARE(s.index(), 2);
}

void TST_RangePresetScaler::pushReturnsMinusOneWhenTheRangeStillFits()
{
	RangePresetScaler s;
	QCOMPARE(s.push(0.05), -1);
	QCOMPARE(s.index(), 0);
}

void TST_RangePresetScaler::settleDropsToTheTightestBracket()
{
	RangePresetScaler s;
	s.push(3.0);
	QCOMPARE(s.index(), 2);

	// First settle still has 3.0 in its window, so it stays.
	QCOMPARE(s.settle(), -1);
	QCOMPARE(s.index(), 2);

	// The window restarted, so the next settle finds the smallest preset fits.
	QCOMPARE(s.settle(), 0);
	QCOMPARE(s.index(), 0);
}

void TST_RangePresetScaler::settleReturnsMinusOneWhenAlreadyTightest()
{
	RangePresetScaler s;
	QCOMPARE(s.settle(), -1);
	QCOMPARE(s.index(), 0);
}

void TST_RangePresetScaler::floorAtZeroClampsTheLowerBound()
{
	RangePresetScaler s;
	s.setFloorAtZero(true);
	QCOMPARE(s.current().lower, 0.0);
	QCOMPARE(s.current().upper, 0.1);

	s.setFloorAtZero(false);
	QCOMPARE(s.current().lower, -0.1);
}

void TST_RangePresetScaler::setPresetsReplacesTheTableAndResets()
{
	RangePresetScaler s;
	s.push(3.0);
	QCOMPARE(s.index(), 2);

	s.setPresets({{-2.0, 2.0, 4, 2}, {-200.0, 200.0, 8, 4}});
	QCOMPARE(s.index(), 0);
	QCOMPARE(s.current().upper, 2.0);
	QCOMPARE(s.push(150.0), 1);

	// An empty table is rejected rather than leaving the scaler unusable.
	s.setPresets({});
	QCOMPARE(s.presets().size(), 2);
}

QTEST_MAIN(TST_RangePresetScaler)

#include "tst_rangepresetscaler.moc"
