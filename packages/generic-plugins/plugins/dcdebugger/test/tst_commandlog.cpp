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
#include <QAction>
#include <QClipboard>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>
#include <QTreeView>
#include <QtTest>

#include <component/commandmonitor.h>
#include <component/connection.h>
#include <core/calltrace.h>
#include <core/pooledcmdexecutor.h>
#include <core/resultcommand.h>
#include <menuonoffswitch.h>
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

// The toggle of the view's "Log commands" switch.
QAbstractButton *logToggle(const CommandLogView &view) { return view.logSwitch()->onOffswitch(); }

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

// A connection with its own executor (and so its own monitor), like the Controller builds them.
struct FakeLink
{
	component::Connection *conn;
	PooledCmdExecutor *exec;

	explicit FakeLink(const QString &uri)
		: conn(new component::Connection(uri, QStringLiteral("fake")))
		, exec(new PooledCmdExecutor(1, conn))
	{
		conn->setExecutor(exec);
	}
	~FakeLink() { delete conn; }
	component::CommandMonitor *monitor() const { return conn->monitor(); }
	void run() const
	{
		TracedCommand cmd;
		exec->execute(&cmd).waitForFinished();
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
	void excludedCommandsAreNotRecorded();
	void excludeIsKeptInPreferences();
	void csvColumns();

	void logSwitchFollowsMonitor();
	void logSwitchDrivesMonitor();
	void recordingFollowsMonitor();
	void twoMonitorsAreIndependent();
	void viewSurvivesMonitorDestruction();
	void disabledRecordsNothingAndKeepsEntries();
	void rowClickRequestsNode();
	void copyTwoRows();
	void copyFoldedRow();
	void copyChildRows();
	void copySkipsFilteredRows();
	void copyEmptySelectionKeepsClipboard();
};

void TST_CommandLog::initTestCase() { CommandLogView::initPreferences(); }

void TST_CommandLog::cleanup() { QVERIFY(!TraceConfig::anyEnabled()); }

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

void TST_CommandLog::excludedCommandsAreNotRecorded()
{
	CommandLogModel m;
	CommandLogEntry ping = entry(QStringLiteral("ctx"), {call("iio_context_get_version")});
	ping.commandClass = QStringLiteral("scopy::iio::PingCommand");
	QCOMPARE(CommandLogModel::commandName(ping), QStringLiteral("PingCommand"));

	m.append(ping);
	m.append(readRaw());
	QCOMPARE(m.entryCount(), 2);

	m.setExcludedCommands({QStringLiteral("PingCommand")});
	QVERIFY(m.isExcluded(QStringLiteral("PingCommand")));
	m.append(ping);
	m.append(readRaw());
	// Dropped on arrival; what was recorded before stays.
	QCOMPARE(m.entryCount(), 3);
	QCOMPARE(m.rowCount(), 3);
	QCOMPARE(m.seenCommands(), QStringList({QStringLiteral("AttrReadCommand"), QStringLiteral("PingCommand")}));

	m.setExcludedCommands({});
	m.append(ping);
	QCOMPARE(m.entryCount(), 4);
}

void TST_CommandLog::excludeIsKeptInPreferences()
{
	Preferences::set(CommandLogView::EXCLUDED_PREF, QStringList());
	{
		CommandLogView view;
		view.setCommandExcluded(QStringLiteral("PingCommand"), true);
		QCOMPARE(view.model()->excludedCommands(), QStringList({QStringLiteral("PingCommand")}));
		QCOMPARE(view.excludeButton()->text(), QStringLiteral("Excluded (1)"));
	}
	QCOMPARE(Preferences::get(CommandLogView::EXCLUDED_PREF).toStringList(),
		 QStringList({QStringLiteral("PingCommand")}));
	{
		// A new log starts with the kept exclusions.
		CommandLogView view;
		QVERIFY(view.model()->isExcluded(QStringLiteral("PingCommand")));
		view.setCommandExcluded(QStringLiteral("PingCommand"), false);
		QVERIFY(!view.model()->isExcluded(QStringLiteral("PingCommand")));
		QCOMPARE(view.excludeButton()->text(), QStringLiteral("Exclude..."));
	}
	QVERIFY(Preferences::get(CommandLogView::EXCLUDED_PREF).toStringList().isEmpty());
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

void TST_CommandLog::logSwitchFollowsMonitor()
{
	FakeLink link(QStringLiteral("ip:a"));
	auto *monitor = link.monitor();
	CommandLogView view;
	QCOMPARE(view.logSwitch()->label()->text(), QStringLiteral("Log commands"));
	// Unbound: nothing to switch.
	QVERIFY(!view.logSwitch()->isEnabled());
	QVERIFY(!logToggle(view)->isChecked());

	view.setMonitor(monitor);
	QCOMPARE(view.monitor(), monitor);
	QVERIFY(view.logSwitch()->isEnabled());
	QVERIFY(!logToggle(view)->isChecked()); // off on a new connection
	QVERIFY(view.statusText().startsWith(QStringLiteral("Logging off")));
	monitor->setEnabled(true); // e.g. from a script
	QVERIFY(logToggle(view)->isChecked());
	QVERIFY(view.statusText().startsWith(QStringLiteral("Logging ·")));
	monitor->setEnabled(false); // e.g. disconnect
	QVERIFY(!logToggle(view)->isChecked());
	QVERIFY(view.statusText().startsWith(QStringLiteral("Logging off")));
}

void TST_CommandLog::logSwitchDrivesMonitor()
{
	FakeLink link(QStringLiteral("ip:a"));
	auto *monitor = link.monitor();
	CommandLogView view;
	view.setMonitor(monitor);
	QSignalSpy spy(monitor, &component::CommandMonitor::enabledChanged);
	logToggle(view)->click();
	QVERIFY(monitor->isEnabled());
	QVERIFY(view.statusText().startsWith(QStringLiteral("Logging ·")));
	logToggle(view)->click();
	QVERIFY(!monitor->isEnabled());
	QCOMPARE(spy.count(), 2);

	// A new view on the same connection starts from the monitor state.
	monitor->setEnabled(true);
	CommandLogView again;
	again.setMonitor(monitor);
	QVERIFY(logToggle(again)->isChecked());
	logToggle(again)->click();
	QVERIFY(!logToggle(view)->isChecked()); // both views follow the shared monitor
}

void TST_CommandLog::recordingFollowsMonitor()
{
	FakeLink link(QStringLiteral("ip:a"));
	auto *monitor = link.monitor();
	CommandLogView view;
	QSignalSpy spy(&view, &CommandLogView::recordingChanged);
	QVERIFY(!view.isRecording());

	// Binding an already recording monitor reports it.
	monitor->setEnabled(true);
	view.setMonitor(monitor);
	QVERIFY(view.isRecording());
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.last()[0].toBool(), true);
	view.setMonitor(monitor); // same monitor: nothing changes
	QCOMPARE(spy.count(), 1);

	// The switch and the monitor both turn it.
	logToggle(view)->click();
	QVERIFY(!view.isRecording());
	QCOMPARE(spy.count(), 2);
	QCOMPARE(spy.last()[0].toBool(), false);
	monitor->setEnabled(true);
	QVERIFY(view.isRecording());
	QCOMPARE(spy.count(), 3);
	QCOMPARE(spy.last()[0].toBool(), true);

	// Unbinding: the view stops reporting, the monitor keeps recording.
	view.setMonitor(nullptr);
	QVERIFY(!view.isRecording());
	QVERIFY(!view.logSwitch()->isEnabled());
	QVERIFY(!logToggle(view)->isChecked());
	QCOMPARE(spy.count(), 4);
	QCOMPARE(spy.last()[0].toBool(), false);
	QVERIFY(monitor->isEnabled());
	monitor->setEnabled(false);
	QCOMPARE(spy.count(), 4);
}

void TST_CommandLog::twoMonitorsAreIndependent()
{
	FakeLink a(QStringLiteral("ip:a"));
	FakeLink b(QStringLiteral("ip:b"));
	CommandLogView viewA;
	CommandLogView viewB;
	viewA.setMonitor(a.monitor());
	viewB.setMonitor(b.monitor());

	logToggle(viewA)->click();
	QVERIFY(a.monitor()->isEnabled());
	QVERIFY(!b.monitor()->isEnabled());
	QVERIFY(!logToggle(viewB)->isChecked());
	a.run();
	b.run();
	QTRY_COMPARE(viewA.model()->entryCount(), 1);
	QTest::qWait(50);
	QCOMPARE(viewB.model()->entryCount(), 0);
	QCOMPARE(viewA.model()->entryAt(0).uri, QStringLiteral("ip:a"));

	logToggle(viewB)->click();
	logToggle(viewA)->click(); // a off, b stays on
	QVERIFY(!a.monitor()->isEnabled());
	QVERIFY(b.monitor()->isEnabled());
	QVERIFY(logToggle(viewB)->isChecked());
	QVERIFY(viewA.statusText().startsWith(QStringLiteral("Logging off")));
	a.run();
	b.run();
	QTRY_COMPARE(viewB.model()->entryCount(), 1);
	QTest::qWait(50);
	QCOMPARE(viewA.model()->entryCount(), 1);
	QCOMPARE(viewB.model()->entryAt(0).uri, QStringLiteral("ip:b"));
	b.monitor()->setEnabled(false);
}

void TST_CommandLog::viewSurvivesMonitorDestruction()
{
	auto *link = new FakeLink(QStringLiteral("ip:a"));
	QPointer<component::CommandMonitor> monitor = link->monitor();
	CommandLogView view;
	view.setMonitor(monitor);
	logToggle(view)->click();
	QVERIFY(view.isRecording());
	QSignalSpy recording(&view, &CommandLogView::recordingChanged);
	link->run();
	QTRY_COMPARE(view.model()->entryCount(), 1);

	delete link; // disconnect: the connection and its monitor go away
	QVERIFY(monitor.isNull());
	QCOMPARE(view.monitor(), nullptr);
	QVERIFY(!view.isRecording());
	QCOMPARE(recording.count(), 1);
	QCOMPARE(recording.last()[0].toBool(), false);
	QVERIFY(!view.logSwitch()->isEnabled());
	QVERIFY(!logToggle(view)->isChecked());
	QVERIFY(view.statusText().startsWith(QStringLiteral("Logging off")));
	QCOMPARE(view.model()->entryCount(), 1); // rows kept (export)
	// Nothing left to drive: no crash.
	logToggle(view)->setChecked(true);
	view.model()->clear();
	QCOMPARE(view.model()->entryCount(), 0);
}

void TST_CommandLog::disabledRecordsNothingAndKeepsEntries()
{
	FakeLink link(QStringLiteral("ip:a"));
	auto *monitor = link.monitor();
	CommandLogView view;
	view.setMonitor(monitor);

	monitor->setEnabled(true);
	link.run();
	QTRY_COMPARE(view.model()->entryCount(), 1);
	QCOMPARE(view.model()->entryAt(0).calls.first().function, QStringLiteral("fake_read"));

	monitor->setEnabled(false);
	link.run();
	QTest::qWait(50);
	QCOMPARE(view.model()->entryCount(), 1); // existing kept, no new

	// Pause: the table is frozen, recording continues.
	monitor->setEnabled(true);
	view.pauseButton()->click();
	QVERIFY(view.model()->isPaused());
	link.run();
	QTRY_COMPARE(view.model()->entryCount(), 2);
	QCOMPARE(view.model()->rowCount(), 1);
	QVERIFY(view.statusText().contains(QStringLiteral("paused")));
	view.pauseButton()->click();
	QCOMPARE(view.model()->rowCount(), 2);
	monitor->setEnabled(false);
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

namespace {
void selectRow(QTreeView *tree, const QModelIndex &index)
{
	tree->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}
} // namespace

void TST_CommandLog::copyTwoRows()
{
	CommandLogView view;
	CommandLogModel *m = view.model();
	m->append(readRaw());
	m->append(entry(QStringLiteral("ctx/dac/scale"), {call("iio_attr_write_raw", -22)}));
	// Selected bottom-up: copied in display order.
	selectRow(view.tree(), m->index(1, 0));
	selectRow(view.tree(), m->index(0, CommandLogModel::ValueColumn));
	const QStringList lines = view.selectedRowsText().split(QLatin1Char('\n'), Qt::SkipEmptyParts);
	QCOMPARE(lines.size(), 2);
	for(int r = 0; r < 2; ++r) {
		const QStringList fields = lines[r].split(QLatin1Char('\t'));
		QCOMPARE(fields.size(), int(CommandLogModel::ColumnCount));
		for(int c = 0; c < CommandLogModel::ColumnCount; ++c) {
			QCOMPARE(fields[c], m->index(r, c).data().toString());
		}
	}
	QCOMPARE(lines[0].split(QLatin1Char('\t'))[CommandLogModel::ComponentColumn], QStringLiteral("ctx/adc/raw"));
	QCOMPARE(lines[1].split(QLatin1Char('\t'))[CommandLogModel::OperationColumn],
		 QStringLiteral("iio_attr_write_raw"));
	QVERIFY(lines[1].split(QLatin1Char('\t'))[CommandLogModel::ValueColumn].startsWith(QStringLiteral("rc=-22")));

	// Smoke: the Ctrl+C action puts the same text on the clipboard.
	QGuiApplication::clipboard()->clear();
	view.copyAction()->trigger();
	QCOMPARE(QGuiApplication::clipboard()->text(), view.selectedRowsText());
	QCOMPARE(view.copyAction()->shortcut(), QKeySequence(QKeySequence::Copy));
	QCOMPARE(view.tree()->selectionMode(), QAbstractItemView::ExtendedSelection);
}

void TST_CommandLog::copyFoldedRow()
{
	CommandLogView view;
	for(int i = 0; i < 10; ++i) {
		view.model()->append(readRaw());
	}
	QCOMPARE(view.model()->rowCount(), 1);
	selectRow(view.tree(), view.model()->index(0, 0));
	const QString text = view.selectedRowsText();
	QVERIFY(text.endsWith(QLatin1Char('\n')));
	const QStringList fields = text.trimmed().split(QLatin1Char('\t'));
	QCOMPARE(fields.size(), int(CommandLogModel::ColumnCount));
	QVERIFY(fields[CommandLogModel::ValueColumn].endsWith(QStringLiteral("×10")));
	QCOMPARE(fields[CommandLogModel::DurationColumn].count(QStringLiteral(" / ")), 2); // min / avg / max
}

void TST_CommandLog::copyChildRows()
{
	CommandLogView view;
	CommandLogModel *m = view.model();
	m->append(entry(QStringLiteral("ctx/adc/InputStream"),
			{call("iio_buffer_open"), call("iio_buffer_stream_start")}));
	const QModelIndex top = m->index(0, 0);
	view.tree()->expand(top);
	selectRow(view.tree(), m->index(1, 0, top));
	selectRow(view.tree(), top);
	QStringList lines = view.selectedRowsText().split(QLatin1Char('\n'), Qt::SkipEmptyParts);
	QCOMPARE(lines.size(), 2);
	QVERIFY(lines[0].contains(QStringLiteral("2 calls")));
	QCOMPARE(lines[1].split(QLatin1Char('\t'))[CommandLogModel::OperationColumn],
		 QStringLiteral("iio_buffer_stream_start"));
	QCOMPARE(view.visibleRowsText().count(QLatin1Char('\n')), 3);

	// Under a collapsed row the child is not shown: not copied.
	view.tree()->collapse(top);
	lines = view.selectedRowsText().split(QLatin1Char('\n'), Qt::SkipEmptyParts);
	QCOMPARE(lines.size(), 1);
	QVERIFY(lines[0].contains(QStringLiteral("2 calls")));
	QCOMPARE(view.visibleRowsText().count(QLatin1Char('\n')), 1);
}

void TST_CommandLog::copySkipsFilteredRows()
{
	CommandLogView view;
	CommandLogModel *m = view.model();
	m->append(entry(QStringLiteral("ctx/adc/raw"), {call("iio_attr_read_raw")}));
	m->append(entry(QStringLiteral("dongle/i2c0"), {call("capi_i2c_transmit")}));
	m->append(entry(QStringLiteral("ctx/adc/scale"), {call("iio_attr_read_raw")}));
	CommandLogFilter f;
	f.component = QStringLiteral("adc");
	m->setFilter(f);
	view.tree()->selectAll();
	const QString text = view.selectedRowsText();
	QCOMPARE(text.count(QLatin1Char('\n')), 2);
	QVERIFY(text.contains(QStringLiteral("ctx/adc/raw")));
	QVERIFY(text.contains(QStringLiteral("ctx/adc/scale")));
	QVERIFY(!text.contains(QStringLiteral("dongle/i2c0")));
	QVERIFY(!view.visibleRowsText().contains(QStringLiteral("dongle/i2c0")));
}

void TST_CommandLog::copyEmptySelectionKeepsClipboard()
{
	CommandLogView view;
	view.model()->append(readRaw());
	QGuiApplication::clipboard()->setText(QStringLiteral("untouched"));
	QVERIFY(view.selectedRowsText().isEmpty());
	view.copyAction()->trigger();
	view.copySelectedRows();
	QCOMPARE(QGuiApplication::clipboard()->text(), QStringLiteral("untouched"));
}

QTEST_MAIN(TST_CommandLog)
#include "tst_commandlog.moc"
