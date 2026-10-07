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

#include "digitaliomath.h"

#include <QTest>

using namespace scopy::adalm2000::dio;

class TST_DigitalIoMath : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void shapeConstantsMatchLegacy();
	void switchOptionOrderPutsTheActiveSenseFirst();
	void bitRoundTripsAndIgnoresOutOfRangePins();
	void groupBaseIsDerivedFromTheIndex();
	void groupValuePacksLsbFirst();
	void withGroupValueLeavesTheOtherGroupUntouched();
	void withGroupValueRoundTripsEveryByte();
	void groupIsOutputOnlyWhenAllEightPinsAre();
	void shortedNeedsOutputEnabledAndADisagreement();
	void shortedIsFalseForInputPins();
};

void TST_DigitalIoMath::shapeConstantsMatchLegacy()
{
	QCOMPARE(PIN_COUNT, 16);
	QCOMPARE(PINS_PER_GROUP, 8);
	QCOMPARE(GROUP_COUNT, 2);
	QCOMPARE(GROUP_VALUE_MAX, 255);
	QCOMPARE(POLL_INTERVAL_MS, 500);
}

// Index 0 is both the switch's first label and its CHECKED state, so it must be
// the active sense; reversing a pair reads as the opposite of the truth.
void TST_DigitalIoMath::switchOptionOrderPutsTheActiveSenseFirst()
{
	QCOMPARE(QString(DIRECTION_OPTIONS[0]), QString("out"));
	QCOMPARE(QString(DIRECTION_OPTIONS[1]), QString("in"));
	QCOMPARE(QString(VALUE_OPTIONS[0]), QString("1"));
	QCOMPARE(QString(VALUE_OPTIONS[1]), QString("0"));
}

void TST_DigitalIoMath::bitRoundTripsAndIgnoresOutOfRangePins()
{
	uint16_t m = 0;
	m = withBit(m, 0, true);
	m = withBit(m, 15, true);
	QCOMPARE(m, uint16_t(0x8001));
	QVERIFY(bit(m, 0));
	QVERIFY(bit(m, 15));
	QVERIFY(!bit(m, 7));

	m = withBit(m, 0, false);
	QCOMPARE(m, uint16_t(0x8000));

	QCOMPARE(withBit(m, 16, true), uint16_t(0x8000));
	QCOMPARE(withBit(m, -1, true), uint16_t(0x8000));
	QVERIFY(!bit(m, 16));
	QVERIFY(!bit(m, -1));
}

void TST_DigitalIoMath::groupBaseIsDerivedFromTheIndex()
{
	QCOMPARE(groupBase(0), 0);
	QCOMPARE(groupBase(1), 8);
	QCOMPARE(groupBase(2), 0);
	QCOMPARE(groupBase(-1), 0);
}

// Bit 0 is the lowest pin in the group.
void TST_DigitalIoMath::groupValuePacksLsbFirst()
{
	QCOMPARE(groupValue(withBit(0, 0, true), 0), uint8_t(0x01));
	QCOMPARE(groupValue(withBit(0, 7, true), 0), uint8_t(0x80));
	QCOMPARE(groupValue(withBit(0, 8, true), 1), uint8_t(0x01));
	QCOMPARE(groupValue(withBit(0, 8, true), 0), uint8_t(0x00));
	QCOMPARE(groupValue(withBit(0, 15, true), 1), uint8_t(0x80));

	QCOMPARE(groupValue(0xA53C, 0), uint8_t(0x3C));
	QCOMPARE(groupValue(0xA53C, 1), uint8_t(0xA5));
}

void TST_DigitalIoMath::withGroupValueLeavesTheOtherGroupUntouched()
{
	const uint16_t start = 0xFFFF;
	QCOMPARE(withGroupValue(start, 0, 0x00), uint16_t(0xFF00));
	QCOMPARE(withGroupValue(start, 1, 0x00), uint16_t(0x00FF));

	QCOMPARE(withGroupValue(0x0000, 0, 0xA5), uint16_t(0x00A5));
	QCOMPARE(withGroupValue(0x0000, 1, 0xA5), uint16_t(0xA500));

	QCOMPARE(withGroupValue(0x1234, 2, 0xFF), uint16_t(0x1234));
}

void TST_DigitalIoMath::withGroupValueRoundTripsEveryByte()
{
	for(int v = 0; v <= GROUP_VALUE_MAX; ++v) {
		const uint8_t value = static_cast<uint8_t>(v);
		for(int g = 0; g < GROUP_COUNT; ++g) {
			QCOMPARE(groupValue(withGroupValue(0x0000, g, value), g), value);
			const int other = 1 - g;
			QCOMPARE(groupValue(withGroupValue(0xFFFF, g, value), other), uint8_t(0xFF));
		}
	}
}

void TST_DigitalIoMath::groupIsOutputOnlyWhenAllEightPinsAre()
{
	QVERIFY(groupIsOutput(0x00FF, 0));
	QVERIFY(groupIsOutput(0xFF00, 1));
	QVERIFY(!groupIsOutput(0x00FF, 1));
	QVERIFY(!groupIsOutput(0x007F, 0));
	QVERIFY(!groupIsOutput(0x0000, 0));
	QVERIFY(!groupIsOutput(0xFFFF, 2));
}

void TST_DigitalIoMath::shortedNeedsOutputEnabledAndADisagreement()
{
	PinState s;
	s.direction = 0x0001; // pin 0 is an output
	s.gpo = 0x0001;	      // being driven high
	s.outputEnabled = true;

	QVERIFY(isShorted(/*gpi=*/0x0000, s, 0));
	QVERIFY(!isShorted(/*gpi=*/0x0001, s, 0));

	// Drivers off: nothing is driven, so a mismatch means nothing.
	s.outputEnabled = false;
	QVERIFY(!isShorted(/*gpi=*/0x0000, s, 0));

	s.outputEnabled = true;
	QVERIFY(!isShorted(0x0000, s, 16));
	QVERIFY(!isShorted(0x0000, s, -1));
}

void TST_DigitalIoMath::shortedIsFalseForInputPins()
{
	PinState s;
	s.direction = 0x0000; // pin 0 is an input
	s.gpo = 0x0001;	      // stale buffered level, never driven
	s.outputEnabled = true;

	// An input pin reads whatever drives it, so disagreeing is normal.
	QVERIFY(!isShorted(/*gpi=*/0x0000, s, 0));
	QVERIFY(!isShorted(/*gpi=*/0x0001, s, 0));
}

QTEST_MAIN(TST_DigitalIoMath)

#include "tst_digitaliomath.moc"
