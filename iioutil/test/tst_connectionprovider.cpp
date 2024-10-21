/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include <iioutil/connectionprovider.h>
#include <iioutil/commandqueue.h>

#include <QTest>

using namespace scopy;

class TestCommandAdd : public Command
{
	Q_OBJECT
public:
	explicit TestCommandAdd(int a, int b, QObject *parent)
		: m_a(a)
		, m_b(b)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual ~TestCommandAdd() { qDebug() << "TestCommand deleted"; }

	virtual void execute() override
	{
		Q_EMIT started(this);
		m_cmdResult->errorCode = m_a + m_b;
		Q_EMIT finished(this);
	}

private:
	int m_a, m_b;
};

class Plugin1 : public QObject
{
	Q_OBJECT
public:
	explicit Plugin1(QString boarduri)
	{
		uri = boarduri;
		conn = ConnectionProvider::open(uri);
		if(conn) {
			connect(conn, &Connection::aboutToBeDestroyed, this, &Plugin1::handleClose);
			commandQueue = conn->commandQueue();
			ctx = conn->context();
		} else {
			conn = nullptr;
			commandQueue = nullptr;
			uri = nullptr;
		}
	}

	explicit Plugin1(struct iio_context *context)
	{
		ctx = context;
		conn = ConnectionProvider::open(ctx);
		if(conn) {
			uri = conn->uri();
			connect(conn, &Connection::aboutToBeDestroyed, this, &Plugin1::handleClose);
			commandQueue = conn->commandQueue();
		} else {
			conn = nullptr;
			commandQueue = nullptr;
			uri = nullptr;
		}
	}

	virtual ~Plugin1() { ConnectionProvider::close(uri); }

	void close() { ConnectionProvider::close(uri); }

	void closeCtx() { ConnectionProvider::close(ctx); }

	void closeConn() { ConnectionProvider::close(conn); }

	void closeAll() { ConnectionProvider::closeAll(uri); }

	void closeAllCtx() { ConnectionProvider::closeAll(ctx); }

	void closeAllConn() { ConnectionProvider::closeAll(conn); }

	const QString &getUri() const { return uri; }

	Connection *getConn() const { return conn; }

	CommandQueue *getCommandQueue() const { return commandQueue; }

	iio_context *getCtx() const { return ctx; }

public Q_SLOTS:
	void handleClose()
	{
		// notify any thread that might be using command queue or ctx
		qInfo() << "Plugin1 handle close on aboutToBeDestroyed() signal";
		conn = nullptr;
		commandQueue = nullptr;
		uri = nullptr;
	}

private:
	QString uri;
	Connection *conn;
	CommandQueue *commandQueue;
	struct iio_context *ctx;
};

class TST_ConnectionProvider : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void testReferenceCount();
	void testMismatchedOpenClose();
	void testSignalAboutToBeDestroyed();
	void testForceClose();
	void testConnectionCmdQ();
	void testOpenCtx();
	void testCloseCtx();
	void testCloseConn();
	void testCloseAllCtx();
	void testCloseAllConn();

private:
	int TEST_A = 100;
	int TEST_B = 20;
};

void TST_ConnectionProvider::testReferenceCount()
{
	QString uri = "ip:192.168.2.1";
	Plugin1 *p1 = new Plugin1(uri);
	if(!p1->getConn()) {
		QSKIP("No context. Skipping");
	}
	QVERIFY2(p1->getConn()->refCount() == 1, "P1 object connection not opened");

	Plugin1 *p2 = new Plugin1(uri);
	QVERIFY2(p1->getConn()->refCount() == 2, "P2 object connection not opened");

	Plugin1 *p3 = new Plugin1(uri);
	QVERIFY2(p1->getConn()->refCount() == 3, "P3 object connection not opened");

	p1->close();
	QVERIFY2(p1->getConn()->refCount() == 2, "P1 object connection not closed");
	p2->close();
	QVERIFY2(p2->getConn()->refCount() == 1, "P2 object connection not closed");
	p3->close();
	QVERIFY2(p3->getConn() == nullptr, "P3 object connection not closed");
}

void TST_ConnectionProvider::testMismatchedOpenClose()
{
	QString uri = "ip:192.168.2.1";
	Plugin1 *p1 = new Plugin1(uri);
	Plugin1 *p2 = new Plugin1(uri);
	if(!p1->getConn()) {
		QSKIP("No context. Skipping");
	}

	QVERIFY2(p1->getConn()->refCount() == 2, "P1 object connection not opened");

	p1->close();
	p1->close();
	QVERIFY2(p1->getConn() == nullptr, "P1 object connection not closed");
	p2->close();
}

void TST_ConnectionProvider::testSignalAboutToBeDestroyed()
{
	QString uri = "ip:192.168.2.1";
	Plugin1 *p1 = new Plugin1(uri);
	Plugin1 *p2 = new Plugin1(uri);
	Plugin1 *p3 = new Plugin1(uri);
	if(!p1->getConn()) {
		QSKIP("No context. Skipping");
	}

	QVERIFY2(p1->getConn() != nullptr, "P1 object connection not opened");
	QVERIFY2(p2->getConn() != nullptr, "P2 object connection not opened");
	QVERIFY2(p3->getConn() != nullptr, "P3 object connection not opened");

	p1->close();
	p2->close();
	p3->close();
	QVERIFY2(p1->getConn() == nullptr, "P1 object connection not closed");
	QVERIFY2(p2->getConn() == nullptr, "P2 object connection not closed");
	QVERIFY2(p3->getConn() == nullptr, "P3 object connection not closed");
}

void TST_ConnectionProvider::testForceClose()
{
	QString uri = "ip:192.168.2.1";
	Plugin1 *p1 = new Plugin1(uri);
	Plugin1 *p2 = new Plugin1(uri);
	Plugin1 *p3 = new Plugin1(uri);
	if(!p1->getConn()) {
		QSKIP("No context. Skipping");
	}

	QVERIFY2(p1->getConn() != nullptr, "P1 object connection not opened");
	QVERIFY2(p2->getConn() != nullptr, "P2 object connection not opened");
	QVERIFY2(p3->getConn() != nullptr, "P3 object connection not opened");

	p3->closeAll();
	QVERIFY2(p1->getConn() == nullptr, "P1 object connection not closed");
	QVERIFY2(p2->getConn() == nullptr, "P2 object connection not closed");
	QVERIFY2(p3->getConn() == nullptr, "P3 object connection not closed");
}

void TST_ConnectionProvider::testConnectionCmdQ()
{
	Command *cmd1 = new TestCommandAdd(TEST_A, TEST_B, nullptr);
	QString uri = "ip:192.168.2.1";
	Plugin1 *p1 = new Plugin1(uri);
	Plugin1 *p2 = new Plugin1(uri);
	Plugin1 *p3 = new Plugin1(uri);
	if(!p1->getConn()) {
		QSKIP("No context. Skipping");
	}

	QVERIFY2(p1->getConn() != nullptr, "P1 object connection not opened");
	QVERIFY2(p2->getConn() != nullptr, "P2 object connection not opened");
	QVERIFY2(p3->getConn() != nullptr, "P3 object connection not opened");

	connect(
		cmd1, &scopy::Command::finished, this,
		[=](scopy::Command *cmd) {
			if(!cmd) {
				cmd = dynamic_cast<Command *>(QObject::sender());
			}
			TestCommandAdd *tcmd = dynamic_cast<TestCommandAdd *>(cmd);
			QVERIFY2(tcmd != nullptr, "Capture command is null");
			QVERIFY2(tcmd == cmd1, "Captured command not the expected one");
			QVERIFY2(tcmd->getReturnCode() == (TEST_B + TEST_A), "TestCommandAdd did not execute");
		},
		Qt::QueuedConnection);

	p1->getCommandQueue()->enqueue(cmd1);

	QTest::qWait(100);
	p3->close();
	p2->close();
	p1->close();
	QVERIFY2(p1->getConn() == nullptr, "P1 object connection not closed");
	QVERIFY2(p2->getConn() == nullptr, "P2 object connection not closed");
	QVERIFY2(p3->getConn() == nullptr, "P3 object connection not closed");
}

void TST_ConnectionProvider::testOpenCtx()
{
	QString uri = "ip:192.168.2.1";
	Plugin1 *p1 = new Plugin1(uri);
	if(!p1->getConn()) {
		QSKIP("No context. Skipping");
	}
	QVERIFY2(p1->getConn() != nullptr, "P1 object connection not opened");

	Plugin1 *p2 = new Plugin1(p1->getCtx());
	QVERIFY2(p2->getConn() != nullptr, "P2 object connection by iio_context not opened");
	QVERIFY2(p2->getUri() == uri, "P2 object connection by iio_context - bad uri");
	QVERIFY2(p1->getConn()->refCount() == 2, "Connection by iio_context and uri not opened");

	p2->close();
	p1->close();
	QVERIFY2(p1->getConn() == nullptr, "P1 object connection not closed");
	QVERIFY2(p2->getConn() == nullptr, "P2 object connection not closed");
}

void TST_ConnectionProvider::testCloseCtx()
{
	QString uri = "ip:192.168.2.1";
	Plugin1 *p1 = new Plugin1(uri);
	Plugin1 *p2 = new Plugin1(uri);
	if(!p1->getConn()) {
		QSKIP("No context. Skipping");
	}

	QVERIFY2(p1->getConn() != nullptr, "P1 object connection not opened");
	QVERIFY2(p2->getConn() != nullptr, "P2 object connection not opened");

	p1->closeCtx();
	QVERIFY2(p1->getConn()->refCount() == 1, "P1 object close by iio_context failed");
	p2->close();
	QVERIFY2(p1->getConn() == nullptr, "P1 object connection not closed");
	QVERIFY2(p2->getConn() == nullptr, "P2 object connection not closed");
}

void TST_ConnectionProvider::testCloseConn()
{
	QString uri = "ip:192.168.2.1";
	Plugin1 *p1 = new Plugin1(uri);
	Plugin1 *p2 = new Plugin1(uri);
	if(!p1->getConn()) {
		QSKIP("No context. Skipping");
	}

	QVERIFY2(p1->getConn() != nullptr, "P1 object connection not opened");
	QVERIFY2(p2->getConn() != nullptr, "P2 object connection not opened");

	p1->closeConn();
	QVERIFY2(p1->getConn()->refCount() == 1, "P1 object close by Connection failed");
	p2->close();
	QVERIFY2(p1->getConn() == nullptr, "P1 object connection not closed");
	QVERIFY2(p2->getConn() == nullptr, "P2 object connection not closed");
}

void TST_ConnectionProvider::testCloseAllCtx()
{
	QString uri = "ip:192.168.2.1";
	Plugin1 *p1 = new Plugin1(uri);
	Plugin1 *p2 = new Plugin1(uri);
	if(!p1->getConn()) {
		QSKIP("No context. Skipping");
	}

	QVERIFY2(p1->getConn() != nullptr, "P1 object connection not opened");
	QVERIFY2(p2->getConn() != nullptr, "P2 object connection not opened");

	p2->closeAllCtx();
	QVERIFY2(p1->getConn() == nullptr, "P1 object connection not closed");
	QVERIFY2(p2->getConn() == nullptr, "P2 object connection not closed");
}

void TST_ConnectionProvider::testCloseAllConn()
{
	QString uri = "ip:192.168.2.1";
	Plugin1 *p1 = new Plugin1(uri);
	Plugin1 *p2 = new Plugin1(uri);
	if(!p1->getConn()) {
		QSKIP("No context. Skipping");
	}

	QVERIFY2(p1->getConn() != nullptr, "P1 object connection not opened");
	QVERIFY2(p2->getConn() != nullptr, "P2 object connection not opened");

	p2->closeAllConn();
	QVERIFY2(p1->getConn() == nullptr, "P1 object connection not closed");
	QVERIFY2(p2->getConn() == nullptr, "P2 object connection not closed");
}

QTEST_MAIN(TST_ConnectionProvider)
#include "tst_connectionprovider.moc"
