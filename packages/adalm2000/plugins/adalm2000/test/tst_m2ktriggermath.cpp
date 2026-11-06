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

#include "m2ktriggermath.h"

#include <QTest>

using namespace scopy::adalm2000::trigger;

class TST_M2kTriggerMath : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void tableSizesMatchTheLibm2kEnums();
	void everyNameRoundTrips();
	void unknownNamesAreRejected();
	void outOfRangeValuesHaveNoName();
	void optionsFollowTheEnumOrder();
	void optionsCanBeTruncated();
	void noTriggerIsTheLastDigitalCondition();

private:
	template <size_t N>
	void verifyRoundTrip(const char *const (&table)[N]);
};

template <size_t N>
void TST_M2kTriggerMath::verifyRoundTrip(const char *const (&table)[N])
{
	for(int i = 0; i < static_cast<int>(N); ++i) {
		const QString name = toName(table, i);
		QVERIFY(!name.isEmpty());
		QCOMPARE(fromName(table, name), i);
	}
}

void TST_M2kTriggerMath::tableSizesMatchTheLibm2kEnums()
{
	QCOMPARE(optionCount(MODE_OPTIONS), 9);
	QCOMPARE(optionCount(ANALOG_CONDITION_OPTIONS), 4);
	QCOMPARE(optionCount(DIGITAL_CONDITION_OPTIONS), 6);
	QCOMPARE(optionCount(ANALOG_SOURCE_OPTIONS), 10);
	QCOMPARE(optionCount(DIGITAL_SOURCE_OPTIONS), 4);
	QCOMPARE(optionCount(OUT_SELECT_OPTIONS), 5);
	QCOMPARE(optionCount(DIGITAL_MODE_OPTIONS), 2);
}

void TST_M2kTriggerMath::everyNameRoundTrips()
{
	verifyRoundTrip(MODE_OPTIONS);
	verifyRoundTrip(ANALOG_CONDITION_OPTIONS);
	verifyRoundTrip(DIGITAL_CONDITION_OPTIONS);
	verifyRoundTrip(ANALOG_SOURCE_OPTIONS);
	verifyRoundTrip(DIGITAL_SOURCE_OPTIONS);
	verifyRoundTrip(OUT_SELECT_OPTIONS);
	verifyRoundTrip(DIGITAL_MODE_OPTIONS);
}

void TST_M2kTriggerMath::unknownNamesAreRejected()
{
	QCOMPARE(fromName(MODE_OPTIONS, QStringLiteral("NOT_A_MODE")), -1);
	QCOMPARE(fromName(MODE_OPTIONS, QString()), -1);
	QCOMPARE(fromName(MODE_OPTIONS, QStringLiteral("always")), -1);
	QCOMPARE(fromName(ANALOG_CONDITION_OPTIONS, QStringLiteral("RISING_EDGE_DIGITAL")), -1);
}

void TST_M2kTriggerMath::outOfRangeValuesHaveNoName()
{
	QVERIFY(toName(MODE_OPTIONS, -1).isEmpty());
	QVERIFY(toName(MODE_OPTIONS, 9).isEmpty());
	QVERIFY(toName(DIGITAL_MODE_OPTIONS, 2).isEmpty());
}

void TST_M2kTriggerMath::optionsFollowTheEnumOrder()
{
	const QStringList options = toOptions(ANALOG_SOURCE_OPTIONS);
	QCOMPARE(options.size(), 10);
	QCOMPARE(options.first(), QStringLiteral("CHANNEL_1"));
	QCOMPARE(options.at(5), QStringLiteral("SRC_DIGITAL_IN"));
	QCOMPARE(options.last(), QStringLiteral("NO_SOURCE"));
}

void TST_M2kTriggerMath::optionsCanBeTruncated()
{
	const QStringList options = toOptions(DIGITAL_CONDITION_OPTIONS, DIGITAL_CONDITION_NONE);
	QCOMPARE(options.size(), 5);
	QVERIFY(!options.contains(QStringLiteral("NO_TRIGGER_DIGITAL")));
	QCOMPARE(toOptions(DIGITAL_MODE_OPTIONS, 99).size(), 2);
}

void TST_M2kTriggerMath::noTriggerIsTheLastDigitalCondition()
{
	QCOMPARE(toName(DIGITAL_CONDITION_OPTIONS, DIGITAL_CONDITION_NONE), QStringLiteral("NO_TRIGGER_DIGITAL"));
}

QTEST_MAIN(TST_M2kTriggerMath)

#include "tst_m2ktriggermath.moc"
