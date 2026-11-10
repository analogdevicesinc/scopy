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

#include "component/ping.h"

#include <QSignalSpy>
#include <QTest>

using namespace scopy;
using namespace scopy::component;

namespace {

// Reports a configurable outcome synchronously, like a backend leaf would once its command ran.
class FakePing : public Ping
{
	Q_OBJECT
public:
	using Ping::Ping;

	bool reachable = true;
	int checks = 0;

	QCoro::Task<CommandResponse<void>> checkReachableAsync() override
	{
		checks++;
		Q_EMIT reachabilityChecked(reachable);
		if(reachable) {
			co_return CommandResponse<void>(QUuid());
		}
		co_return CommandResponse<void>(QUuid(), Unexpected<Error>{Error{-EIO, "unreachable"}});
	}
};

} // namespace

class TstComponentPing : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void defaults();
	void startStopSignals();
	void setMonitoringProperty();
	void intervalWhileRunningRestarts();
	void intervalWhileStoppedIsStored();
	void intervalClamped();
	void startStoresInterval();
	void resultRecorded();
	void failureStopsOnce();
	void failureWhileStoppedKeepsState();
	void restartClearsLost();
};

void TstComponentPing::defaults()
{
	FakePing p;
	QCOMPARE(Ping::DEFAULT_INTERVAL_MS, 2000);
	QCOMPARE(p.intervalMs(), 2000);
	QVERIFY(!p.isMonitoring());
	QVERIFY(!p.hasResult());
	QVERIFY(!p.isLost());
	QVERIFY(!p.lastCheckedAt().isValid());
	QCOMPARE(p.property("intervalMs").toInt(), 2000);
	QCOMPARE(p.property("monitoring").toBool(), false);

	QSignalSpy interval(&p, &Ping::intervalChanged);
	p.startMonitoring(); // default argument = DEFAULT_INTERVAL_MS: unchanged
	QCOMPARE(p.intervalMs(), 2000);
	QCOMPARE(interval.count(), 0);
}

void TstComponentPing::startStopSignals()
{
	FakePing p;
	QSignalSpy mon(&p, &Ping::monitoringChanged);
	p.startMonitoring();
	QVERIFY(p.isMonitoring());
	QCOMPARE(mon.count(), 1);
	QCOMPARE(mon.at(0).at(0).toBool(), true);

	p.startMonitoring(); // already running: no change
	QCOMPARE(mon.count(), 1);

	p.stopMonitoring();
	QVERIFY(!p.isMonitoring());
	QCOMPARE(mon.count(), 2);
	QCOMPARE(mon.at(1).at(0).toBool(), false);

	p.stopMonitoring(); // already stopped: no change
	QCOMPARE(mon.count(), 2);
}

void TstComponentPing::setMonitoringProperty()
{
	FakePing p;
	p.setIntervalMs(300);
	QSignalSpy mon(&p, &Ping::monitoringChanged);
	QVERIFY(p.setProperty("monitoring", true));
	QVERIFY(p.isMonitoring());
	QCOMPARE(p.intervalMs(), 300); // starts with the stored interval
	QVERIFY(p.setProperty("monitoring", false));
	QVERIFY(!p.isMonitoring());
	QCOMPARE(mon.count(), 2);
}

void TstComponentPing::intervalWhileRunningRestarts()
{
	FakePing p;
	p.startMonitoring(60000);
	QCOMPARE(p.checks, 0);
	QSignalSpy interval(&p, &Ping::intervalChanged);
	QSignalSpy mon(&p, &Ping::monitoringChanged);
	p.setIntervalMs(100);
	QCOMPARE(interval.count(), 1);
	QCOMPARE(interval.at(0).at(0).toInt(), 100);
	QVERIFY(p.isMonitoring());
	QCOMPARE(mon.count(), 0); // a restart is not a state change
	// The timer now runs at the new period (it would not fire for a minute otherwise).
	QTRY_VERIFY_WITH_TIMEOUT(p.checks >= 2, 3000);

	p.setIntervalMs(100); // same value: nothing
	QCOMPARE(interval.count(), 1);
}

void TstComponentPing::intervalWhileStoppedIsStored()
{
	FakePing p;
	QSignalSpy interval(&p, &Ping::intervalChanged);
	QSignalSpy mon(&p, &Ping::monitoringChanged);
	p.setIntervalMs(100);
	QCOMPARE(p.intervalMs(), 100);
	QCOMPARE(interval.count(), 1);
	QVERIFY(!p.isMonitoring());
	QCOMPARE(mon.count(), 0);
	QTest::qWait(300);
	QCOMPARE(p.checks, 0);
}

void TstComponentPing::intervalClamped()
{
	FakePing p;
	p.setIntervalMs(5);
	QCOMPARE(p.intervalMs(), Ping::MIN_INTERVAL_MS);
	p.setIntervalMs(-1);
	QCOMPARE(p.intervalMs(), Ping::MIN_INTERVAL_MS);
}

void TstComponentPing::startStoresInterval()
{
	FakePing p;
	QSignalSpy interval(&p, &Ping::intervalChanged);
	p.startMonitoring(5000);
	QCOMPARE(p.intervalMs(), 5000);
	QCOMPARE(interval.count(), 1);
	QCOMPARE(interval.at(0).at(0).toInt(), 5000);
	QCOMPARE(p.property("intervalMs").toInt(), 5000);
}

void TstComponentPing::resultRecorded()
{
	FakePing p;
	QSignalSpy result(&p, &Ping::resultChanged);
	const QDateTime before = QDateTime::currentDateTime();
	p.checkReachableAsync();
	QCOMPARE(result.count(), 1);
	QVERIFY(p.hasResult());
	QVERIFY(p.lastReachable());
	QVERIFY(!p.isLost());
	QVERIFY(p.lastCheckedAt().isValid());
	QVERIFY(p.lastCheckedAt() >= before);
}

void TstComponentPing::failureStopsOnce()
{
	FakePing p;
	p.startMonitoring(100);
	QSignalSpy mon(&p, &Ping::monitoringChanged);
	QSignalSpy lost(&p, &Ping::connectionLost);
	QSignalSpy result(&p, &Ping::resultChanged);
	p.reachable = false;
	p.checkReachableAsync();
	QVERIFY(p.isLost());
	QVERIFY(!p.isMonitoring());
	QVERIFY(p.hasResult());
	QVERIFY(!p.lastReachable());
	QCOMPARE(lost.count(), 1);
	QCOMPARE(mon.count(), 1);
	QCOMPARE(mon.at(0).at(0).toBool(), false);
	QCOMPARE(result.count(), 1);

	// Further failures update the result but neither re-stop nor re-lose.
	p.checkReachableAsync();
	QCOMPARE(lost.count(), 1);
	QCOMPARE(mon.count(), 1);
	QCOMPARE(result.count(), 2);
	QTest::qWait(250);
	QCOMPARE(p.checks, 2); // the timer stopped on the first failure
}

void TstComponentPing::failureWhileStoppedKeepsState()
{
	FakePing p;
	QSignalSpy mon(&p, &Ping::monitoringChanged);
	QSignalSpy lost(&p, &Ping::connectionLost);
	p.reachable = false;
	p.checkReachableAsync();
	QVERIFY(p.isLost());
	QCOMPARE(lost.count(), 1);
	QCOMPARE(mon.count(), 0); // it was not running
}

void TstComponentPing::restartClearsLost()
{
	FakePing p;
	p.startMonitoring(60000);
	p.reachable = false;
	p.checkReachableAsync();
	QVERIFY(p.isLost());

	QSignalSpy mon(&p, &Ping::monitoringChanged);
	QSignalSpy result(&p, &Ping::resultChanged);
	QSignalSpy lost(&p, &Ping::connectionLost);
	p.startMonitoring(60000);
	QVERIFY(!p.isLost());
	QVERIFY(p.isMonitoring());
	QCOMPARE(mon.count(), 1);
	QCOMPARE(result.count(), 1);
	// The last result is kept.
	QVERIFY(p.hasResult());
	QVERIFY(!p.lastReachable());

	// A new failure is reported again.
	p.checkReachableAsync();
	QCOMPARE(lost.count(), 1);
	QVERIFY(!p.isMonitoring());
}

QTEST_MAIN(TstComponentPing)
#include "tst_component_ping.moc"
