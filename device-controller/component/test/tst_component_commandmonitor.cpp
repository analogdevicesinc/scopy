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
#include "component/connection.h"
#include "component/context.h"
#include "component/controller.h"
#include "component/device.h"

#include "core/calltrace.h"
#include "core/pooledcmdexecutor.h"
#include "core/resultcommand.h"

#include <QMutex>
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <QWaitCondition>
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

// Blocks inside run() until released (teardown while a traced command is in flight).
class GatedCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	GatedCommand()
		: ResultCommand(nullptr)
	{}

	void waitEntered()
	{
		QMutexLocker lock(&m_mutex);
		while(!m_entered) {
			m_cond.wait(&m_mutex);
		}
	}
	void release()
	{
		QMutexLocker lock(&m_mutex);
		m_open = true;
		m_cond.wakeAll();
	}

protected:
	void run() override
	{
		DC_TRACE("fakelib", "fake_before", nullptr, fakeTransfer(0));
		{
			QMutexLocker lock(&m_mutex);
			m_entered = true;
			m_cond.wakeAll();
			while(!m_open) {
				m_cond.wait(&m_mutex);
			}
		}
		DC_TRACE("fakelib", "fake_after", nullptr, fakeTransfer(0));
		setResult(Result<void>());
	}

private:
	QMutex m_mutex;
	QWaitCondition m_cond;
	bool m_entered = false;
	bool m_open = false;
};

// A connection with its own executor and one named context, like the Controller builds them.
struct FakeLink
{
	Connection *conn;
	Context *ctx;
	PooledCmdExecutor *exec;

	explicit FakeLink(const QString &uri)
		: conn(new Connection(uri, QStringLiteral("fake")))
		, ctx(new Context())
		, exec(new PooledCmdExecutor(1, conn))
	{
		conn->setExecutor(exec);
		ctx->setName(QStringLiteral("fakectx"));
		ctx->setUri(uri);
		conn->add(QStringLiteral("fake"), ctx);
	}
	~FakeLink()
	{
		// Like the Controller: views go before their connection.
		delete conn->take(QStringLiteral("fake"));
		delete conn;
	}
	CommandMonitor *monitor() const { return conn->monitor(); }
};

} // namespace

class TstComponentCommandMonitor : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void cleanup() { QVERIFY(!TraceConfig::anyEnabled()); }

	void oneMonitorPerConnection();
	void enabledFollowsConfig();
	void oneLeafTwoCallsOneEntry();
	void deliveredQueuedToGuiThread();
	void disabledEmitsNothing();
	void twoConnectionsAreIndependent();
	void connectionDestroyedWhileInFlight();
	void monitorGoesWithControllerConnection();
	void originSetByAttributeReader();
	void originOfUnnamedLeaf();
};

void TstComponentCommandMonitor::oneMonitorPerConnection()
{
	FakeLink a(QStringLiteral("fake:a"));
	FakeLink b(QStringLiteral("fake:b"));
	QVERIFY(a.monitor());
	QVERIFY(b.monitor());
	QVERIFY(a.monitor() != b.monitor());
	QCOMPARE(a.monitor()->parent(), a.conn);
	QCOMPARE(a.monitor()->connection(), a.conn);
	QCOMPARE(a.monitor()->uri(), QStringLiteral("fake:a"));
	QCOMPARE(a.conn->findChildren<CommandMonitor *>().size(), 1);
	// Installed on the connection's executor.
	QCOMPARE(a.exec->traceConfig(), a.monitor()->config());
	QVERIFY(!a.monitor()->isEnabled()); // off by default

	// Resolves from the connection, a context and any node below it.
	auto *dev = new Device(a.ctx);
	auto *leaf = new QObject(dev);
	QCOMPARE(CommandMonitor::of(a.conn), a.monitor());
	QCOMPARE(CommandMonitor::of(a.ctx), a.monitor());
	QCOMPARE(CommandMonitor::of(leaf), a.monitor());
	QCOMPARE(CommandMonitor::of(b.ctx), b.monitor());
	QObject orphan;
	QCOMPARE(CommandMonitor::of(&orphan), nullptr);
	QCOMPARE(CommandMonitor::of(nullptr), nullptr);

	ConnectionHandle h(QStringLiteral("fake:not-registered"), a.conn);
	QCOMPARE(h.monitor(), a.monitor());
	QCOMPARE(ConnectionHandle().monitor(), nullptr);
}

void TstComponentCommandMonitor::enabledFollowsConfig()
{
	FakeLink link(QStringLiteral("fake:0"));
	CommandMonitor *m = link.monitor();
	QSignalSpy spy(m, &CommandMonitor::enabledChanged);
	QVERIFY(!m->isEnabled());
	m->setEnabled(true);
	QVERIFY(m->config()->isEnabled());
	QVERIFY(TraceConfig::anyEnabled());
	m->setEnabled(true); // no change, no signal
	m->setEnabled(false);
	QVERIFY(!m->config()->isEnabled());
	QCOMPARE(spy.count(), 2);
	QCOMPARE(spy[0][0].toBool(), true);
	QCOMPARE(spy[1][0].toBool(), false);

	QSignalSpy capture(m, &CommandMonitor::captureDataChanged);
	m->setCaptureData(true);
	m->setCaptureData(true);
	QVERIFY(m->config()->captureData());
	QCOMPARE(capture.count(), 1);
	m->setPreviewBytes(8);
	QCOMPARE(m->config()->previewBytes(), 8);
}

void TstComponentCommandMonitor::oneLeafTwoCallsOneEntry()
{
	FakeLink link(QStringLiteral("fake:0"));
	CommandMonitor *m = link.monitor();
	auto *dev = new Device(link.ctx);
	dev->setName("dev0");
	auto *leaf = new TwoCallLeaf(link.exec, dev);

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
	m->setEnabled(false);
}

void TstComponentCommandMonitor::deliveredQueuedToGuiThread()
{
	FakeLink link(QStringLiteral("fake:0"));
	CommandMonitor *m = link.monitor();
	auto *leaf = new TwoCallLeaf(link.exec, link.ctx);

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
	m->setEnabled(false);
}

void TstComponentCommandMonitor::disabledEmitsNothing()
{
	FakeLink link(QStringLiteral("fake:0"));
	CommandMonitor *m = link.monitor();
	auto *leaf = new TwoCallLeaf(link.exec, link.ctx);

	QSignalSpy spy(m, &CommandMonitor::commandFinished);
	leaf->start().waitForFinished();
	QTest::qWait(50);
	QCOMPARE(spy.count(), 0);
}

void TstComponentCommandMonitor::twoConnectionsAreIndependent()
{
	FakeLink a(QStringLiteral("fake:a"));
	FakeLink b(QStringLiteral("fake:b"));
	auto *leafA = new TwoCallLeaf(a.exec, a.ctx);
	auto *leafB = new TwoCallLeaf(b.exec, b.ctx);
	QSignalSpy spyA(a.monitor(), &CommandMonitor::commandFinished);
	QSignalSpy spyB(b.monitor(), &CommandMonitor::commandFinished);
	QSignalSpy enabledB(b.monitor(), &CommandMonitor::enabledChanged);
	auto runBoth = [&]() {
		leafA->start().waitForFinished();
		leafB->start().waitForFinished();
		QTest::qWait(50);
	};

	a.monitor()->setEnabled(true);
	QVERIFY(!b.monitor()->isEnabled());
	QCOMPARE(enabledB.count(), 0);
	runBoth();
	QCOMPARE(spyA.count(), 1);
	QCOMPARE(spyB.count(), 0);
	QCOMPARE(spyA[0][0].value<CommandLogEntry>().uri, QStringLiteral("fake:a"));

	b.monitor()->setEnabled(true);
	runBoth();
	QCOMPARE(spyA.count(), 2);
	QCOMPARE(spyB.count(), 1);
	QCOMPARE(spyB[0][0].value<CommandLogEntry>().uri, QStringLiteral("fake:b"));

	// Turning a off leaves b on.
	a.monitor()->setEnabled(false);
	QVERIFY(b.monitor()->isEnabled());
	runBoth();
	QCOMPARE(spyA.count(), 2);
	QCOMPARE(spyB.count(), 2);
	b.monitor()->setEnabled(false);
}

void TstComponentCommandMonitor::connectionDestroyedWhileInFlight()
{
	auto *conn = new Connection(QStringLiteral("fake:0"), QStringLiteral("fake"));
	auto *exec = new PooledCmdExecutor(1, conn);
	conn->setExecutor(exec);
	CommandMonitor *m = conn->monitor();
	QPointer<CommandMonitor> guard(m);
	std::weak_ptr<TraceConfig> config = m->config();
	m->setEnabled(true);

	int delivered = 0;
	QObject receiver;
	connect(m, &CommandMonitor::commandFinished, &receiver, [&delivered](const CommandLogEntry &) { ++delivered; });

	GatedCommand cmd;
	QFuture<void> f = exec->execute(&cmd);
	cmd.waitEntered();
	// The monitor goes first (detached), then the executor drains: let the command finish in
	// between, i.e. during the connection's teardown.
	connect(m, &QObject::destroyed, &receiver, [&cmd]() { cmd.release(); });
	delete conn;
	QVERIFY(f.isFinished());
	QVERIFY(guard.isNull());
	QVERIFY(config.expired());
	QVERIFY(cmd.isTraced()); // started on: complete entry, never delivered
	QCOMPARE(cmd.calls().size(), 2);
	QTest::qWait(50);
	QCOMPARE(delivered, 0);
}

void TstComponentCommandMonitor::monitorGoesWithControllerConnection()
{
	auto *ctx = new Context();
	ctx->setName(QStringLiteral("adopted"));
	ContextHandle h = Controller::GetInstance()->adopt(QStringLiteral("fake:adopted"), ctx);
	QVERIFY(h);
	QPointer<CommandMonitor> m = CommandMonitor::of(h.get());
	QVERIFY(m);
	QCOMPARE(m.data(), h.connection()->monitor());
	m->setEnabled(true);
	h.reset();
	QTRY_VERIFY(m.isNull());
}

void TstComponentCommandMonitor::originSetByAttributeReader()
{
	dctest::FakeAttrOps ops;
	auto handle = ops.make("raw", "42");
	auto *conn = new Connection(QStringLiteral("ip:test"), QStringLiteral("iio"));
	auto *exec = new PooledCmdExecutor(1, conn);
	conn->setExecutor(exec);
	CommandMonitor *m = conn->monitor();

	auto *ctx = new Context();
	ctx->setName("ctx0");
	conn->add(QStringLiteral("iio"), ctx);
	auto *dev = new Device(ctx);
	dev->setName("adc");
	auto *attr = new Attribute(dev);
	attr->setName("raw");
	auto *reader = new component::iio::IIOAttributeReader(&ops, handle, exec);
	attr->addReadCapability(reader);

	m->setEnabled(true);
	QSignalSpy spy(m, &CommandMonitor::commandFinished);
	QCoro::waitFor(reader->readAsync());
	QTRY_COMPARE(spy.count(), 1);
	const auto e = spy[0][0].value<CommandLogEntry>();
	// The reader resolves to its Attribute.
	QCOMPARE(e.origin, QStringLiteral("ctx0/adc/raw"));
	QCOMPARE(e.backendId, QStringLiteral("iio"));
	QCOMPARE(e.uri, QStringLiteral("ip:test"));
	QVERIFY(e.commandClass.contains(QStringLiteral("AttrReadCommand")));
	delete conn->take(QStringLiteral("iio"));
	delete conn;
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
