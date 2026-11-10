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

#include "fakeattrops.h"

#include "component/attribute.h"
#include "component/backends/iio/iioattributereader.h"
#include "component/commandmonitor.h"
#include "component/context.h"
#include "component/device.h"

#include "core/calltrace.h"
#include "core/pooledcmdexecutor.h"
#include "core/resultcommand.h"

#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component;

namespace {

int fakeTransfer(int n) { return n; }

// One command issuing two library calls (like buffer open = create + start).
class TwoCallCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	TwoCallCommand()
		: ResultCommand(nullptr)
	{}

protected:
	void run() override
	{
		DC_TRACE("fakelib", "fake_open", reinterpret_cast<void *>(0x100), fakeTransfer(0),
			 CallArg::string("device", QStringLiteral("dev0")));
		DC_TRACE("fakelib", "fake_start", reinterpret_cast<void *>(0x100), fakeTransfer(0),
			 CallArg::integer("samples", 1024));
		setResult(Result<void>());
	}
};

// A backend leaf: tags and dispatches its command like the IIO leaves do.
class TwoCallLeaf : public QObject
{
	Q_OBJECT
public:
	TwoCallLeaf(ICmdExecutor *exec, QObject *parent)
		: QObject(parent)
		, m_exec(exec)
	{}

	QFuture<void> start()
	{
		auto *cmd = new TwoCallCommand();
		CommandMonitor::tag(cmd, this);
		QFuture<void> f = m_exec->execute(cmd);
		f.then([cmd] { cmd->deleteLater(); });
		return f;
	}

private:
	ICmdExecutor *m_exec;
};

} // namespace

class TstComponentCommandMonitor : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init() { CommandMonitor::GetInstance()->setEnabled(false); }
	void cleanup() { CommandMonitor::GetInstance()->setEnabled(false); }

	void enabledFollowsCallTrace();
	void oneLeafTwoCallsOneEntry();
	void deliveredQueuedToGuiThread();
	void disabledEmitsNothing();
	void originSetByAttributeReader();
	void originOfUnnamedLeaf();
};

void TstComponentCommandMonitor::enabledFollowsCallTrace()
{
	CommandMonitor *m = CommandMonitor::GetInstance();
	QSignalSpy spy(m, &CommandMonitor::enabledChanged);
	QVERIFY(!m->isEnabled());
	m->setEnabled(true);
	QVERIFY(CallTrace::isEnabled());
	m->setEnabled(true); // no change, no signal
	m->setEnabled(false);
	QVERIFY(!CallTrace::isEnabled());
	QCOMPARE(spy.count(), 2);
	QCOMPARE(spy[0][0].toBool(), true);
	QCOMPARE(spy[1][0].toBool(), false);
}

void TstComponentCommandMonitor::oneLeafTwoCallsOneEntry()
{
	CommandMonitor *m = CommandMonitor::GetInstance();
	PooledCmdExecutor exec(1);
	Context ctx;
	ctx.setName("fakectx");
	ctx.setBackend("fake");
	ctx.setUri("fake:0");
	auto *dev = new Device(&ctx);
	dev->setName("dev0");
	auto *leaf = new TwoCallLeaf(&exec, dev);

	m->setEnabled(true);
	QSignalSpy spy(m, &CommandMonitor::commandFinished);
	leaf->start();
	QVERIFY(spy.wait());
	QTest::qWait(50);
	QCOMPARE(spy.count(), 1);

	const auto e = spy[0][0].value<CommandLogEntry>();
	QCOMPARE(e.calls.size(), 2);
	QCOMPARE(e.calls[0].function, QStringLiteral("fake_open"));
	QCOMPARE(e.calls[1].function, QStringLiteral("fake_start"));
	QCOMPARE(e.calls[0].library, QStringLiteral("fakelib"));
	QCOMPARE(e.origin, QStringLiteral("fakectx/dev0/TwoCallLeaf"));
	QCOMPARE(e.originObject, quintptr(leaf));
	QCOMPARE(e.backendId, QStringLiteral("fake"));
	QCOMPARE(e.uri, QStringLiteral("fake:0"));
	QVERIFY(e.commandClass.contains(QStringLiteral("TwoCallCommand")));
	QVERIFY(!e.failed());
}

void TstComponentCommandMonitor::deliveredQueuedToGuiThread()
{
	CommandMonitor *m = CommandMonitor::GetInstance();
	PooledCmdExecutor exec(1);
	QObject root;
	auto *leaf = new TwoCallLeaf(&exec, &root);

	QThread *deliveredOn = nullptr;
	QObject receiver;
	connect(m, &CommandMonitor::commandFinished, &receiver,
		[&deliveredOn](const CommandLogEntry &) { deliveredOn = QThread::currentThread(); });

	m->setEnabled(true);
	leaf->start().waitForFinished();
	// Emitted on the executor thread: nothing is delivered before the GUI loop runs.
	QCOMPARE(deliveredOn, nullptr);
	QTRY_VERIFY(deliveredOn != nullptr);
	QCOMPARE(deliveredOn, QThread::currentThread());
}

void TstComponentCommandMonitor::disabledEmitsNothing()
{
	CommandMonitor *m = CommandMonitor::GetInstance();
	PooledCmdExecutor exec(1);
	QObject root;
	auto *leaf = new TwoCallLeaf(&exec, &root);

	QSignalSpy spy(m, &CommandMonitor::commandFinished);
	leaf->start().waitForFinished();
	QTest::qWait(50);
	QCOMPARE(spy.count(), 0);
}

void TstComponentCommandMonitor::originSetByAttributeReader()
{
	CommandMonitor *m = CommandMonitor::GetInstance();
	dctest::FakeAttrOps ops;
	auto handle = ops.make("raw", "42");
	PooledCmdExecutor exec(1);

	Context ctx;
	ctx.setName("ctx0");
	ctx.setBackend("iio");
	auto *dev = new Device(&ctx);
	dev->setName("adc");
	auto *attr = new Attribute(dev);
	attr->setName("raw");
	auto *reader = new component::iio::IIOAttributeReader(&ops, handle, &exec);
	attr->addReadCapability(reader);

	m->setEnabled(true);
	QSignalSpy spy(m, &CommandMonitor::commandFinished);
	QCoro::waitFor(reader->readAsync());
	QTRY_COMPARE(spy.count(), 1);
	const auto e = spy[0][0].value<CommandLogEntry>();
	// The reader resolves to its Attribute.
	QCOMPARE(e.origin, QStringLiteral("ctx0/adc/raw"));
	QCOMPARE(e.backendId, QStringLiteral("iio"));
	QVERIFY(e.commandClass.contains(QStringLiteral("AttrReadCommand")));
}

void TstComponentCommandMonitor::originOfUnnamedLeaf()
{
	Context ctx;
	ctx.setName("ctx0");
	auto *dev = new Device(&ctx);
	dev->setName("adc");
	auto *leaf = new QObject(dev);
	QCOMPARE(CommandMonitor::originOf(leaf).path, QStringLiteral("ctx0/adc/QObject"));
	QCOMPARE(CommandMonitor::originOf(dev).path, QStringLiteral("ctx0/adc"));
	QCOMPARE(CommandMonitor::originOf(dev).object, quintptr(dev));
}

QTEST_MAIN(TstComponentCommandMonitor)
#include "tst_component_commandmonitor.moc"
