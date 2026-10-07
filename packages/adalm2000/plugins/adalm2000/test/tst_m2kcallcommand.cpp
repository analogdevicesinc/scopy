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

#include "m2kcallcommand.h"

#include <QTest>
#include <stdexcept>
#include <string>

using namespace scopy;
using namespace scopy::adalm2000;

class TST_M2kCallCommand : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void successStoresValue();
	void voidSuccessHasValue();
	void throwingCallableYieldsError();
	void throwingCallableDoesNotPropagate();
	void errorMessageIsOurs();
	void cancelledCommandDoesNotRun();
};

void TST_M2kCallCommand::successStoresValue()
{
	M2kCallCommand<int> cmd(nullptr, []() { return 42; });
	cmd.execute();
	auto r = cmd.result();
	QVERIFY(bool(r));
	QCOMPARE(r.value(), 42);
}

void TST_M2kCallCommand::voidSuccessHasValue()
{
	bool ran = false;
	M2kCallCommand<void> cmd(nullptr, [&ran]() { ran = true; });
	cmd.execute();
	QVERIFY(ran);
	QVERIFY(bool(cmd.result()));
}

void TST_M2kCallCommand::throwingCallableYieldsError()
{
	M2kCallCommand<int> cmd(nullptr, []() -> int { throw std::runtime_error("boom"); });
	cmd.execute();
	auto r = cmd.result();
	QVERIFY(!bool(r));
	QCOMPARE(r.error().code, -EIO);
}

void TST_M2kCallCommand::throwingCallableDoesNotPropagate()
{
	M2kCallCommand<void> cmd(nullptr, []() { throw std::runtime_error("boom"); });
	bool returnedNormally = false;
	cmd.execute();
	returnedNormally = true;
	QVERIFY(returnedNormally);
	QVERIFY(!bool(cmd.result()));
}

void TST_M2kCallCommand::errorMessageIsOurs()
{
	const std::string libm2kText = "ERR: Invalid argument - Channel: Cannot write done_led_overwrite_powerdown";
	M2kCallCommand<bool> cmd(nullptr, [&libm2kText]() -> bool { throw std::runtime_error(libm2kText); });
	cmd.execute();
	const QString msg = cmd.result().error().message;
	QVERIFY(msg.startsWith("libm2k call failed"));
	QVERIFY(msg.contains(QString::fromStdString(libm2kText)));
}

void TST_M2kCallCommand::cancelledCommandDoesNotRun()
{
	bool ran = false;
	M2kCallCommand<void> cmd(nullptr, [&ran]() { ran = true; });
	cmd.cancel();
	cmd.execute();
	QVERIFY(!ran);
	QVERIFY(!bool(cmd.result()));
}

QTEST_MAIN(TST_M2kCallCommand)

#include "tst_m2kcallcommand.moc"
