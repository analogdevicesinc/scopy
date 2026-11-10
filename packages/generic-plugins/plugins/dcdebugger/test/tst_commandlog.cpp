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

#include "log/commandlogmodel.h"
#include "log/commandlogview.h"

#include <QAbstractButton>
#include <QPushButton>
#include <QSignalSpy>
#include <QTreeView>
#include <QtTest>

#include <component/commandmonitor.h>
#include <core/calltrace.h>
#include <core/pooledcmdexecutor.h>
#include <core/resultcommand.h>
#include <pluginbase/preferences.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {

CallRecord call(const QString &fn, int rc = 0, const QString &lib = QStringLiteral("libiio"), quintptr handle = 0x10)
{
	CallRecord c;
	c.library = lib;
	c.function = fn;
	c.handle = handle;
	c.args = {CallArg::string("attr", QStringLiteral("raw"))};
	c.rc = rc;
	c.durationNs = 1000;
	return c;
}

CommandLogEntry entry(const QString &origin, const QList<CallRecord> &calls,
		      const QString &uri = QStringLiteral("ip:a"))
{
	CommandLogEntry e;
	e.origin = origin;
	e.uri = uri;
	e.backendId = QStringLiteral("iio");
	e.commandClass = QStringLiteral("scopy::iio::AttrReadCommand");
	e.startNs = 1'700'000'000'000'000'000LL;
	e.durationNs = 2000;
	e.calls = calls;
	for(const CallRecord &c : calls) {
		if(c.rc < 0) {
			e.error = Error{c.rc, QStringLiteral("failed")};
		}
	}
	return e;
}

CommandLogEntry readRaw(int rc = 4) { return entry(QStringLiteral("ctx/adc/raw"), {call("iio_attr_read_raw", rc)}); }

int fakeCall(int v) { return v; }

// One traced library call, like an attribute read.
class TracedCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	TracedCommand()
		: ResultCommand(nullptr)
	{}

protected:
	void run() override
	{
		DC_TRACE("fakelib", "fake_read", nullptr, fakeCall(1));
		setResult(Result<void>());
	}
};

} // namespace

class TST_CommandLog : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void initTestCase();
	void cleanup();

	void ringBufferDropsOldest();
	void nineIdenticalAreNineRows();
	void tenIdenticalFoldIntoOne();
	void rcChangeEndsTheRun();
	void collapseNever();
	void multiCallChildren();
	void dropFromFoldedRow();
	void filters();
	void uriFilter();
	void pauseKeepsRecording();
	void csvColumns();

	void switchFollowsMonitor();
	void switchDrivesMonitorAndPreference();
	void disabledRecordsNothingAndKeepsEntries();
	void rowClickRequestsNode();
};

void TST_CommandLog::initTestCase()
{
	CommandLogView::initPreferences();
	component::CommandMonitor::GetInstance()->setEnabled(false);
}

void TST_CommandLog::cleanup() { component::CommandMonitor::GetInstance()->setEnabled(false); }

void TST_CommandLog::ringBufferDropsOldest()
{
	CommandLogModel m;
	m.setCapacity(5);
	for(int i = 0; i < 8; ++i) {
		m.append(entry(QStringLiteral("dev/attr%1").arg(i), {call("iio_attr_read_raw")}));
	}
	QCOMPARE(m.entryCount(), 5);
	QCOMPARE(m.entryAt(0).origin, QStringLiteral("dev/attr3"));
	QCOMPARE(m.rowCount(), 5);
	QCOMPARE(m.index(0, CommandLogModel::ComponentColumn).data().toString(), QStringLiteral("dev/attr3"));
	m.setCapacity(2);
	QCOMPARE(m.entryCount(), 2);
	QCOMPARE(m.rowCount(), 2);
}

void TST_CommandLog::nineIdenticalAreNineRows()
{
	CommandLogModel m;
	QCOMPARE(m.collapseAfter(), 10);
	for(int i = 0; i < 9; ++i) {
		m.append(readRaw());
	}
	QCOMPARE(m.rowCount(), 9);
	QCOMPARE(m.index(8, 0).data(CommandLogModel::RepeatRole).toInt(), 1);
}

void TST_CommandLog::tenIdenticalFoldIntoOne()
{
	CommandLogModel m;
	for(int i = 0; i < 10; ++i) {
		m.append(readRaw());
	}
	QCOMPARE(m.rowCount(), 1);
	const QModelIndex row = m.index(0, CommandLogModel::ValueColumn);
	QCOMPARE(row.data(CommandLogModel::RepeatRole).toInt(), 10);
	QVERIFY(row.data().toString().contains(QStringLiteral("×10")));
	QCOMPARE(m.index(0, CommandLogModel::OperationColumn).data().toString(), QStringLiteral("iio_attr_read_raw"));
	QVERIFY(m.index(0, CommandLogModel::DurationColumn).data().toString().contains(QLatin1Char('/')));
	// Expandable: every folded command is still there.
	QCOMPARE(m.rowCount(m.index(0, 0)), 10);
	m.append(readRaw());
	QCOMPARE(m.rowCount(), 1);
	QCOMPARE(m.index(0, 0).data(CommandLogModel::RepeatRole).toInt(), 11);
	QCOMPARE(m.entryCount(), 11);
}

void TST_CommandLog::rcChangeEndsTheRun()
{
	CommandLogModel m;
	for(int i = 0; i < 10; ++i) {
		m.append(readRaw());
	}
	m.append(readRaw(-5));
	QCOMPARE(m.rowCount(), 2);
	QVERIFY(m.index(1, 0).data(CommandLogModel::FailedRole).toBool());
	QVERIFY(m.index(1, 0).data(Qt::ForegroundRole).isValid());
	QVERIFY(!m.index(0, 0).data(CommandLogModel::FailedRole).toBool());
	// The run starts over: plain rows again until it reaches 10.
	for(int i = 0; i < 9; ++i) {
		m.append(readRaw());
	}
	QCOMPARE(m.rowCount(), 11);
	m.append(readRaw());
	QCOMPARE(m.rowCount(), 3);
	QCOMPARE(m.index(2, 0).data(CommandLogModel::RepeatRole).toInt(), 10);

	// A different call (same rc) ends the run too.
	CommandLogModel m2;
	for(int i = 0; i < 5; ++i) {
		m2.append(readRaw());
	}
	m2.append(entry(QStringLiteral("ctx/adc/raw"), {call("iio_attr_write_raw", 4)}));
	for(int i = 0; i < 5; ++i) {
		m2.append(readRaw());
	}
	QCOMPARE(m2.rowCount(), 11);
}

void TST_CommandLog::collapseNever()
{
	CommandLogModel m;
	m.setCollapseAfter(0);
	for(int i = 0; i < 20; ++i) {
		m.append(readRaw());
	}
	QCOMPARE(m.rowCount(), 20);
	m.setCollapseAfter(10);
	QCOMPARE(m.rowCount(), 1);
	QCOMPARE(m.index(0, 0).data(CommandLogModel::RepeatRole).toInt(), 20);
}

void TST_CommandLog::multiCallChildren()
{
	CommandLogModel m;
	m.append(entry(
		QStringLiteral("ctx/adc/InputStream"),
		{call("iio_buffer_open"), call("iio_buffer_stream_create_block"), call("iio_buffer_stream_start")}));
	QCOMPARE(m.rowCount(), 1);
	const QModelIndex top = m.index(0, 0);
	QVERIFY(m.index(0, CommandLogModel::OperationColumn).data().toString().contains(QStringLiteral("3 calls")));
	QCOMPARE(m.rowCount(top), 3);
	QCOMPARE(m.index(2, CommandLogModel::OperationColumn, top).data().toString(),
		 QStringLiteral("iio_buffer_stream_start"));
	QCOMPARE(m.parent(m.index(1, 0, top)), top);
	// One-call rows have no children.
	m.append(readRaw());
	QCOMPARE(m.rowCount(m.index(1, 0)), 0);
}

void TST_CommandLog::dropFromFoldedRow()
{
	CommandLogModel m;
	m.setCapacity(12);
	for(int i = 0; i < 12; ++i) {
		m.append(readRaw());
	}
	m.append(entry(QStringLiteral("ctx/adc/other"), {call("iio_attr_read_raw")}));
	QCOMPARE(m.entryCount(), 12);
	QCOMPARE(m.rowCount(), 2);
	QCOMPARE(m.index(0, 0).data(CommandLogModel::RepeatRole).toInt(), 11);
	QCOMPARE(m.rowCount(m.index(0, 0)), 11);
}

void TST_CommandLog::filters()
{
	CommandLogModel m;
	m.append(entry(QStringLiteral("ctx/adc/raw"), {call("iio_attr_read_raw")}));
	m.append(entry(QStringLiteral("ctx/adc/scale"), {call("iio_attr_write_raw")}));
	m.append(entry(QStringLiteral("dongle/i2c0"), {call("capi_i2c_transmit", -110, QStringLiteral("remote-bus"))}));
	m.append(entry(QStringLiteral("ctx/dac/raw"), {call("iio_attr_read_raw")}));
	QCOMPARE(m.rowCount(), 4);

	CommandLogFilter f;
	f.component = QStringLiteral("ADC");
	m.setFilter(f);
	QCOMPARE(m.rowCount(), 2);

	f = {};
	f.library = QStringLiteral("remote");
	m.setFilter(f);
	QCOMPARE(m.rowCount(), 1);
	QCOMPARE(m.index(0, CommandLogModel::ComponentColumn).data().toString(), QStringLiteral("dongle/i2c0"));

	f = {};
	f.function = QStringLiteral("read_raw");
	m.setFilter(f);
	QCOMPARE(m.rowCount(), 2);

	f = {};
	f.errorsOnly = true;
	m.setFilter(f);
	QCOMPARE(m.rowCount(), 1);
	QVERIFY(m.index(0, 0).data(CommandLogModel::FailedRole).toBool());

	// New entries respect the active filter; clearing it shows everything buffered.
	m.append(readRaw());
	QCOMPARE(m.rowCount(), 1);
	m.setFilter({});
	QCOMPARE(m.rowCount(), 5);
	QCOMPARE(m.filteredEntries().size(), 5);
}

void TST_CommandLog::uriFilter()
{
	CommandLogModel m;
	m.setUri(QStringLiteral("ip:a"));
	m.append(entry(QStringLiteral("x"), {call("f")}, QStringLiteral("ip:a")));
	m.append(entry(QStringLiteral("y"), {call("f")}, QStringLiteral("ip:b")));
	QCOMPARE(m.entryCount(), 1);
}

void TST_CommandLog::pauseKeepsRecording()
{
	CommandLogModel m;
	m.append(readRaw());
	m.setPaused(true);
	for(int i = 0; i < 3; ++i) {
		m.append(entry(QStringLiteral("ctx/adc/x%1").arg(i), {call("iio_attr_read_raw")}));
	}
	QCOMPARE(m.rowCount(), 1);
	QCOMPARE(m.entryCount(), 4);
	m.setPaused(false);
	QCOMPARE(m.rowCount(), 4);
}

void TST_CommandLog::csvColumns()
{
	CommandLogModel m;
	m.append(readRaw());
	m.append(entry(QStringLiteral("ctx/adc/InputStream"),
		       {call("iio_buffer_open"), call("iio_buffer_stream_start")}));
	m.append(entry(QStringLiteral("ctx/adc/x"), {}));
	const QStringList lines = m.toCsv().split(QLatin1Char('\n'), Qt::SkipEmptyParts);
	const QStringList expected = {
		"time",	  "uri",  "backend", "component", "command",	"library",     "function",
		"handle", "args", "rc",	     "value",	  "value_size", "duration_ns", "command_duration_ns",
		"error"};
	QCOMPARE(CommandLogModel::csvColumns(), expected);
	QCOMPARE(lines.first(), expected.join(QLatin1Char(',')));
	// One line per call; an entry without calls still gets one.
	QCOMPARE(lines.size(), 1 + 1 + 2 + 1);
	QVERIFY(lines[1].contains(QStringLiteral("iio_attr_read_raw")));
	QVERIFY(lines[1].contains(QStringLiteral("ctx/adc/raw")));
	QCOMPARE(lines[1].split(QLatin1Char(',')).size(), expected.size());
}

void TST_CommandLog::switchFollowsMonitor()
{
	CommandLogView view;
	auto *monitor = component::CommandMonitor::GetInstance();
	QVERIFY(!view.logSwitch()->isChecked());
	QVERIFY(view.statusText().startsWith(QStringLiteral("Logging off")));
	Preferences::set(CommandLogView::ENABLED_PREF, false);
	monitor->setEnabled(true); // e.g. from a script
	QVERIFY(view.logSwitch()->isChecked());
	QVERIFY(view.statusText().startsWith(QStringLiteral("Logging ·")));
	// Following the monitor is not a user choice: the preference is untouched.
	QCOMPARE(Preferences::get(CommandLogView::ENABLED_PREF).toBool(), false);
	monitor->setEnabled(false); // e.g. disconnect
	QVERIFY(!view.logSwitch()->isChecked());
}

void TST_CommandLog::switchDrivesMonitorAndPreference()
{
	CommandLogView view;
	auto *monitor = component::CommandMonitor::GetInstance();
	view.logSwitch()->click();
	QVERIFY(monitor->isEnabled());
	QVERIFY(CommandLogView::enabledPreference());
	view.logSwitch()->click();
	QVERIFY(!monitor->isEnabled());
	QVERIFY(!CommandLogView::enabledPreference());

	// A new view (next connect) starts from the monitor state.
	monitor->setEnabled(true);
	CommandLogView again;
	QVERIFY(again.logSwitch()->isChecked());
}

void TST_CommandLog::disabledRecordsNothingAndKeepsEntries()
{
	CommandLogView view;
	auto *monitor = component::CommandMonitor::GetInstance();
	PooledCmdExecutor exec(1);
	auto run = [&exec]() {
		auto *cmd = new TracedCommand();
		exec.execute(cmd).waitForFinished();
		delete cmd;
	};

	monitor->setEnabled(true);
	run();
	QTRY_COMPARE(view.model()->entryCount(), 1);
	QCOMPARE(view.model()->entryAt(0).calls.first().function, QStringLiteral("fake_read"));

	monitor->setEnabled(false);
	run();
	QTest::qWait(50);
	QCOMPARE(view.model()->entryCount(), 1); // existing kept, no new

	// Pause: the table is frozen, recording continues.
	monitor->setEnabled(true);
	view.pauseButton()->click();
	QVERIFY(view.model()->isPaused());
	run();
	QTRY_COMPARE(view.model()->entryCount(), 2);
	QCOMPARE(view.model()->rowCount(), 1);
	QVERIFY(view.statusText().contains(QStringLiteral("paused")));
	view.pauseButton()->click();
	QCOMPARE(view.model()->rowCount(), 2);
}

void TST_CommandLog::rowClickRequestsNode()
{
	CommandLogView view;
	CommandLogEntry e = readRaw();
	e.originObject = 0x1234;
	view.model()->append(e);
	QSignalSpy spy(&view, &CommandLogView::nodeRequested);
	Q_EMIT view.tree()->clicked(view.model()->index(0, CommandLogModel::ValueColumn));
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy[0][0].toString(), QStringLiteral("ctx/adc/raw"));
	QCOMPARE(spy[0][1].toString(), QStringLiteral("iio"));
	QCOMPARE(spy[0][2].value<quintptr>(), quintptr(0x1234));
}

QTEST_MAIN(TST_CommandLog)
#include "tst_commandlog.moc"
