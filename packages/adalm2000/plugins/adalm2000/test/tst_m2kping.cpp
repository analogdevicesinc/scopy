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

#include "m2kping.h"

#include "core/pooledcmdexecutor.h"

#include <QSignalSpy>
#include <QTest>
#include <qcoro/qcorofuture.h>

#include <stdexcept>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::adalm2000;

class TST_M2kPing : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void ledTrueIsReachable();
	void ledFalseIsAlsoReachable();
	void pingReportsUnreachableWhenReaderThrows();
	void connectionLostIsEmittedOnceAndStopsTheTimer();
	void monitoringCallsTheReader();
};

void TST_M2kPing::ledTrueIsReachable()
{
	PooledCmdExecutor exec(1);
	M2kPing ping(
		nullptr, []() { return true; }, &exec);

	QSignalSpy spy(&ping, &Ping::reachabilityChecked);
	QVERIFY(bool(QCoro::waitFor(ping.checkReachableAsync())));
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.at(0).at(0).toBool(), true);
}

void TST_M2kPing::ledFalseIsAlsoReachable()
{
	PooledCmdExecutor exec(1);
	M2kPing ping(
		nullptr, []() { return false; }, &exec);

	QSignalSpy reach(&ping, &Ping::reachabilityChecked);
	QSignalSpy lost(&ping, &Ping::connectionLost);
	QVERIFY(bool(QCoro::waitFor(ping.checkReachableAsync())));
	QCOMPARE(reach.count(), 1);
	QCOMPARE(reach.at(0).at(0).toBool(), true);
	QCOMPARE(lost.count(), 0);
}

void TST_M2kPing::pingReportsUnreachableWhenReaderThrows()
{
	PooledCmdExecutor exec(1);
	M2kPing ping(
		nullptr, []() -> bool { throw std::runtime_error("ERR: Invalid argument - Channel"); }, &exec);

	QSignalSpy reach(&ping, &Ping::reachabilityChecked);
	const auto resp = QCoro::waitFor(ping.checkReachableAsync());
	QVERIFY(!bool(resp));
	QCOMPARE(reach.count(), 1);
	QCOMPARE(reach.at(0).at(0).toBool(), false);
}

void TST_M2kPing::connectionLostIsEmittedOnceAndStopsTheTimer()
{
	PooledCmdExecutor exec(1);
	M2kPing ping(
		nullptr, []() -> bool { throw std::runtime_error("gone"); }, &exec);

	QSignalSpy lost(&ping, &Ping::connectionLost);
	ping.startMonitoring(20);
	QTRY_COMPARE(lost.count(), 1);
	QVERIFY(!ping.isMonitoring());
	QTest::qWait(100);
	QCOMPARE(lost.count(), 1);
}

void TST_M2kPing::monitoringCallsTheReader()
{
	PooledCmdExecutor exec(1);
	std::atomic_int calls{0};
	M2kPing ping(
		nullptr,
		[&calls]() {
			++calls;
			return true;
		},
		&exec);

	ping.startMonitoring(20);
	QTRY_VERIFY(calls.load() >= 2);
	ping.stopMonitoring();
}

QTEST_MAIN(TST_M2kPing)

#include "tst_m2kping.moc"
