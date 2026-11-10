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

#include "nodepanel.h"
#include "ping/pingview.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTabWidget>
#include <QtTest>

#include <component/ping.h>
#include <qcoro/qcorotask.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {

// Reports a configurable outcome, like a backend leaf once its command ran.
class FakePing : public component::Ping
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

struct Ui
{
	QCheckBox *monitoring;
	QSpinBox *period;
	QPushButton *pingNow;
	QLabel *result;
	QLabel *checkedAt;
	QLabel *lost;
};

Ui ui(QWidget *view)
{
	Ui u{view->findChild<QCheckBox *>("pingMonitoring"), view->findChild<QSpinBox *>("pingPeriod"),
	     view->findChild<QPushButton *>("pingNow"),	     view->findChild<QLabel *>("pingResult"),
	     view->findChild<QLabel *>("pingCheckedAt"),     view->findChild<QLabel *>("pingLost")};
	return u;
}

void setPeriod(QSpinBox *spin, int ms)
{
	spin->setFocus();
	spin->selectAll();
	QTest::keyClicks(spin, QString::number(ms));
	QTest::keyClick(spin, Qt::Key_Return);
}

} // namespace

class TST_PingView : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void initialStopped();
	void initialStarted();
	void periodRange();
	void externalChangesFollowed();
	void checkboxDrivesMonitoring();
	void spinboxDrivesInterval();
	void typingDoesNotApply();
	void pingNowShowsResult();
	void lostShownOnlyWhenLost();
	void survivesPingDestruction();
	void nodePanelHasPingTab();
};

void TST_PingView::initialStopped()
{
	FakePing p;
	PingView v(&p);
	const Ui u = ui(&v);
	QVERIFY(u.monitoring && u.period && u.pingNow && u.result && u.checkedAt && u.lost);
	QVERIFY(!u.monitoring->isChecked());
	QCOMPARE(u.period->value(), component::Ping::DEFAULT_INTERVAL_MS);
	QCOMPARE(u.result->text(), QStringLiteral("—"));
	QCOMPARE(u.checkedAt->text(), QStringLiteral("—"));
	QVERIFY(u.lost->isHidden());
	QVERIFY(u.monitoring->toolTip().contains("disables automatic disconnect detection"));
	// Opening the panel imposes nothing.
	QVERIFY(!p.isMonitoring());
	QCOMPARE(p.checks, 0);
}

void TST_PingView::initialStarted()
{
	FakePing p;
	p.startMonitoring(5000);
	PingView v(&p);
	const Ui u = ui(&v);
	QVERIFY(u.monitoring->isChecked());
	QCOMPARE(u.period->value(), 5000);
	QVERIFY(p.isMonitoring());
	QCOMPARE(p.intervalMs(), 5000);
}

void TST_PingView::periodRange()
{
	FakePing p;
	PingView v(&p);
	const Ui u = ui(&v);
	QCOMPARE(u.period->minimum(), 100);
	QCOMPARE(u.period->maximum(), 60000);
	QCOMPARE(u.period->singleStep(), 100);
	QCOMPARE(u.period->suffix(), QStringLiteral(" ms"));
}

void TST_PingView::externalChangesFollowed()
{
	FakePing p;
	p.startMonitoring();
	PingView v(&p);
	const Ui u = ui(&v);
	QVERIFY(u.monitoring->isChecked());
	QSignalSpy mon(&p, &component::Ping::monitoringChanged);

	p.stopMonitoring(); // e.g. another plugin pausing it during an acquisition
	QVERIFY(!u.monitoring->isChecked());
	p.setIntervalMs(700);
	QCOMPARE(u.period->value(), 700);
	p.startMonitoring(1500);
	QVERIFY(u.monitoring->isChecked());
	QCOMPARE(u.period->value(), 1500);
	// Following the Ping does not write back to it.
	QCOMPARE(mon.count(), 2);
	QCOMPARE(p.intervalMs(), 1500);
}

void TST_PingView::checkboxDrivesMonitoring()
{
	FakePing p;
	p.setIntervalMs(800);
	PingView v(&p);
	const Ui u = ui(&v);
	QSignalSpy mon(&p, &component::Ping::monitoringChanged);
	u.monitoring->click();
	QVERIFY(p.isMonitoring());
	QCOMPARE(p.intervalMs(), 800); // started with the current interval
	u.monitoring->click();
	QVERIFY(!p.isMonitoring());
	QCOMPARE(mon.count(), 2);
}

void TST_PingView::spinboxDrivesInterval()
{
	FakePing p;
	PingView v(&p);
	v.show();
	QVERIFY(QTest::qWaitForWindowExposed(&v));
	const Ui u = ui(&v);

	setPeriod(u.period, 3000);
	QCOMPARE(p.intervalMs(), 3000);
	QVERIFY(!p.isMonitoring()); // stopped: only stored

	p.startMonitoring(3000);
	u.period->stepUp();
	QCOMPARE(p.intervalMs(), 3100);
	QVERIFY(p.isMonitoring());
}

void TST_PingView::typingDoesNotApply()
{
	FakePing p;
	PingView v(&p);
	v.show();
	QVERIFY(QTest::qWaitForWindowExposed(&v));
	const Ui u = ui(&v);
	QSignalSpy interval(&p, &component::Ping::intervalChanged);
	u.period->setFocus();
	u.period->selectAll();
	QTest::keyClicks(u.period, "4500");
	QCOMPARE(interval.count(), 0); // not while typing
	QCOMPARE(p.intervalMs(), component::Ping::DEFAULT_INTERVAL_MS);
	QTest::keyClick(u.period, Qt::Key_Return);
	QCOMPARE(interval.count(), 1);
	QCOMPARE(p.intervalMs(), 4500);

	// Out of range input is corrected to the nearest bound when committed.
	setPeriod(u.period, 45);
	QCOMPARE(p.intervalMs(), 100);
	QCOMPARE(u.period->value(), 100);
}

void TST_PingView::pingNowShowsResult()
{
	FakePing p;
	PingView v(&p);
	const Ui u = ui(&v);
	u.pingNow->click();
	QCOMPARE(p.checks, 1);
	QCOMPARE(u.result->text(), QStringLiteral("Reachable"));
	QVERIFY(QTime::fromString(u.checkedAt->text().section(' ', 1), "HH:mm:ss.zzz").isValid());
	QVERIFY(u.lost->isHidden());
	QTRY_VERIFY(u.pingNow->isEnabled());
	QVERIFY(!p.isMonitoring()); // a one-off check does not start monitoring
}

void TST_PingView::lostShownOnlyWhenLost()
{
	FakePing p;
	p.startMonitoring();
	PingView v(&p);
	const Ui u = ui(&v);
	p.reachable = false;
	p.checkReachableAsync();
	QCOMPARE(u.result->text(), QStringLiteral("Unreachable"));
	QVERIFY(!u.lost->isHidden());
	QVERIFY(!u.monitoring->isChecked()); // the automatic stop is shown

	p.reachable = true;
	p.startMonitoring(); // a restart clears lost; the last result stays
	QVERIFY(u.lost->isHidden());
	QCOMPARE(u.result->text(), QStringLiteral("Unreachable"));
	QVERIFY(u.monitoring->isChecked());
	p.checkReachableAsync();
	QCOMPARE(u.result->text(), QStringLiteral("Reachable"));
	QVERIFY(u.lost->isHidden());
}

void TST_PingView::survivesPingDestruction()
{
	auto *p = new FakePing;
	p->startMonitoring();
	PingView v(p);
	const Ui u = ui(&v);
	delete p;
	QTRY_VERIFY(!u.monitoring->isEnabled());
	QVERIFY(!v.ping());
	QVERIFY(!u.period->isEnabled());
	QVERIFY(!u.pingNow->isEnabled());
	// The controls are inert now.
	u.monitoring->setEnabled(true);
	u.monitoring->click();
	u.period->setValue(900);
	u.pingNow->setEnabled(true);
	u.pingNow->click();
	v.refresh();
	QCOMPARE(u.result->text(), QStringLiteral("—"));
}

void TST_PingView::nodePanelHasPingTab()
{
	FakePing p;
	NodePanel *panel = createNodePanel(&p, NodeKind::Ping, "ping", {}, {});
	QVERIFY(panel->findChild<PingView *>());
	auto *tabs = panel->findChild<QTabWidget *>();
	QVERIFY(tabs);
	QCOMPARE(tabs->tabText(0), QStringLiteral("Ping"));
	delete panel;
}

QTEST_MAIN(TST_PingView)
#include "tst_pingview.moc"
