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

#include "fakestream.h"
#include "monitor/attributemonitor.h"
#include "nodeactivity.h"

#include <QSignalSpy>
#include <QtTest>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

class TST_AttributeMonitor : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void parsesLeadingNumbers();
	void pollsOnlyEnabledItems();
	void scaledRawUsesOffsetAndScale();
	void historyIsTrimmed();
	void statsOverHistory();
	void readErrorsAreReported();
	void activityFollowsRunningMonitors();
};

void TST_AttributeMonitor::parsesLeadingNumbers()
{
	double v = 0;
	QVERIFY(AttributeMonitor::parseValue("25.13", v) && v == 25.13);
	QVERIFY(AttributeMonitor::parseValue(" 71.000000 dB", v) && v == 71.0);
	QVERIFY(AttributeMonitor::parseValue("-1.5e3", v) && v == -1500);
	QVERIFY(!AttributeMonitor::parseValue("slow_attack", v));
	QVERIFY(!AttributeMonitor::parseValue("", v));
}

void TST_AttributeMonitor::pollsOnlyEnabledItems()
{
	QObject root;
	FakeAttributeReader *ra = nullptr;
	FakeAttributeReader *rb = nullptr;
	AttributeMonitor mon;
	mon.addItem("a", makeAttribute(&root, "input", "1", &ra));
	mon.addItem("b", makeAttribute(&root, "input", "2", &rb));
	mon.setPeriod(5);
	mon.setEnabled("a", true);
	mon.start();
	QTRY_VERIFY(mon.samples("a").size() >= 3);
	QCOMPARE(rb->reads, 0);
	QCOMPARE(mon.enabledKeys(), QStringList({"a"}));
	mon.stop();
	QVERIFY(!mon.isRunning());
}

void TST_AttributeMonitor::scaledRawUsesOffsetAndScale()
{
	QObject root;
	Attribute *scale = makeAttribute(&root, "scale", "0.5");
	Attribute *offset = makeAttribute(&root, "offset", "-10");
	AttributeMonitor mon;
	mon.addItem("raw", makeAttribute(&root, "raw", "30"), scale, offset);
	mon.setEnabled("raw", true);
	mon.setPeriod(5);
	mon.start();
	QTRY_VERIFY(!mon.samples("raw").isEmpty());
	QCOMPARE(mon.stats("raw").value, 10.0); // (30 - 10) * 0.5
	mon.setScaled(false);
	QVERIFY(mon.samples("raw").isEmpty()); // history restarts with the units
	QTRY_VERIFY(!mon.samples("raw").isEmpty());
	QCOMPARE(mon.stats("raw").value, 30.0);
}

void TST_AttributeMonitor::historyIsTrimmed()
{
	QObject root;
	AttributeMonitor mon;
	mon.addItem("a", makeAttribute(&root, "input", "1"));
	mon.setEnabled("a", true);
	mon.setPeriod(5);
	mon.setHistory(0.1);
	mon.start();
	QTest::qWait(400);
	const QVector<QPointF> s = mon.samples("a");
	QVERIFY(!s.isEmpty());
	QVERIFY(s.last().x() - s.first().x() <= 0.1 + 0.02);
}

void TST_AttributeMonitor::statsOverHistory()
{
	QObject root;
	FakeAttributeReader *r = nullptr;
	AttributeMonitor mon;
	mon.addItem("a", makeAttribute(&root, "input", "1", &r));
	mon.setEnabled("a", true);
	mon.setPeriod(5);
	mon.start();
	QTRY_VERIFY(mon.samples("a").size() >= 2);
	r->value = "3";
	QTRY_COMPARE(mon.stats("a").value, 3.0);
	const AttributeMonitor::Stats st = mon.stats("a");
	QCOMPARE(st.min, 1.0);
	QCOMPARE(st.max, 3.0);
	QVERIFY(st.mean > 1.0 && st.mean < 3.0);
	mon.clear();
	QCOMPARE(mon.stats("a").count, 0);
}

void TST_AttributeMonitor::readErrorsAreReported()
{
	QObject root;
	FakeAttributeReader *r = nullptr;
	AttributeMonitor mon;
	mon.addItem("a", makeAttribute(&root, "input", "x", &r));
	mon.setEnabled("a", true);
	QSignalSpy failed(&mon, &AttributeMonitor::readFailed);
	mon.start();
	QTRY_VERIFY(!failed.isEmpty());
	QVERIFY(mon.lastError("a").startsWith("not a number"));
	r->fail = true;
	QTRY_VERIFY(mon.lastError("a") == "read failed");
}

void TST_AttributeMonitor::activityFollowsRunningMonitors()
{
	NodeActivity *activity = NodeActivity::instance();
	QObject node;
	QSignalSpy changed(activity, &NodeActivity::activeChanged);
	auto *a = new AttributeMonitor();
	auto *b = new AttributeMonitor();
	activity->track(&node, a);
	activity->track(&node, b);
	QVERIFY(!activity->isActive(&node));

	a->start();
	QVERIFY(activity->isActive(&node));
	b->start(); // already active: no second signal
	QCOMPARE(changed.count(), 1);
	QCOMPARE(changed.at(0).at(1).toBool(), true);

	a->stop();
	QVERIFY(activity->isActive(&node)); // b still runs
	delete b;			    // a destroyed running monitor counts as stopped
	QVERIFY(!activity->isActive(&node));
	QCOMPARE(changed.count(), 2);
	QCOMPARE(changed.at(1).at(1).toBool(), false);

	// Tracked while already running.
	a->start();
	QObject other;
	activity->track(&other, a);
	QVERIFY(activity->isActive(&other));
	delete a;
	QVERIFY(!activity->isActive(&node));
	QVERIFY(!activity->isActive(&other));
}

QTEST_GUILESS_MAIN(TST_AttributeMonitor)
#include "tst_attributemonitor.moc"
