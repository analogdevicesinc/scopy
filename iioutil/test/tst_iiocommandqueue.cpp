#include "iioutil/command.h"
#include "iioutil/commandqueue.h"

#include <QPushButton>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QVector>
#include <QtConcurrent/QtConcurrent>

using namespace scopy;
Q_DECLARE_METATYPE(QSignalSpy *);
Q_DECLARE_METATYPE(CommandQueue *);

class TestCommandCounter : public Command
{
	Q_OBJECT
public:
	explicit TestCommandCounter(QObject *parent)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual ~TestCommandCounter() { qDebug() << "TestCommandCounter deleted"; }

	virtual void execute() override { m_commandCounter++; }

public:
	static std::atomic<int> m_commandCounter;
};
std::atomic<int> TestCommandCounter::m_commandCounter = 0;

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

class TestCommandMultiply : public Command
{
	Q_OBJECT
public:
	explicit TestCommandMultiply(int a, int b, QObject *parent)
		: m_a(a)
		, m_b(b)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual ~TestCommandMultiply() { qDebug() << "TestCommandMultiply deleted"; }

	virtual void execute() override
	{
		Q_EMIT started(this);
		m_cmdResult->errorCode = m_a * m_b;
		const char *tmp = "TEST OK";
		m_cmdResult->results = (char *)tmp;
		Q_EMIT finished(this);
	}

	std::string getResult() { return std::string((char *)m_cmdResult->results); }

private:
	int m_a, m_b;
	std::string m_msg;
};

class TestCommandMsg : public Command
{
	Q_OBJECT
public:
	explicit TestCommandMsg(int sleep, QString message, QObject *parent)
		: m_sleep(sleep)
		, m_msg_btn(new QPushButton(message))
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual ~TestCommandMsg()
	{
		qDebug() << "TestCommandMsg deleted";
		if(m_cmdResult->results) {
			delete(QPushButton *)m_cmdResult->results;
			m_cmdResult->results = nullptr;
		}
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		QThread::msleep(m_sleep);
		m_cmdResult->errorCode = m_sleep;
		m_cmdResult->results = (void *)m_msg_btn;
		Q_EMIT finished(this);
	}

	QPushButton *getResult() { return static_cast<QPushButton *>(m_cmdResult->results); }

private:
	int m_sleep;
	QPushButton *m_msg_btn;
};

class TST_IioCommandQueue : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void testResults();
	void testCommandOrder();
	//	void testLaunchCommandFromThread();
private:
	int TEST_A = 100;
	int TEST_B = 20;
	QThreadPool m_commandExecThreadPool;
};

void TST_IioCommandQueue::testResults()
{
	CommandQueue *cmdQ = new CommandQueue(1, nullptr);
	Command *cmd1 = new TestCommandAdd(TEST_A, TEST_B, nullptr);
	Command *cmd2 = new TestCommandMultiply(TEST_A, TEST_B, nullptr);
	Command *cmd3 = new TestCommandMsg(TEST_A, "Test command 300", nullptr);

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

	connect(
		cmd2, &scopy::Command::finished, this,
		[=](scopy::Command *cmd) {
			if(!cmd) {
				cmd = dynamic_cast<Command *>(QObject::sender());
			}
			TestCommandMultiply *tcmd = dynamic_cast<TestCommandMultiply *>(cmd);
			QVERIFY2(tcmd != nullptr, "Capture command is null");
			QVERIFY2(tcmd == cmd2, "Captured command not the expected one");
			QVERIFY2(tcmd->getResult() == "TEST OK", "TestCommandMultiply not executed properly");
			QVERIFY2(tcmd->getReturnCode() == (TEST_B * TEST_A), "TestCommandMultiply did not execute");
		},
		Qt::QueuedConnection);

	connect(
		cmd3, &scopy::Command::finished, this,
		[=](scopy::Command *cmd) {
			if(!cmd) {
				cmd = dynamic_cast<Command *>(QObject::sender());
			}
			TestCommandMsg *tcmd = dynamic_cast<TestCommandMsg *>(cmd);
			QVERIFY2(tcmd != nullptr, "Capture command is null");
			QVERIFY2(tcmd == cmd3, "Captured command not the expected one");
			QVERIFY2(tcmd->getReturnCode() == TEST_A, "TestCommandMsg did not execute");
			QPushButton *testBtn = tcmd->getResult();
			QVERIFY2(testBtn != nullptr, "TestCommandMsg did not generate results");
			qDebug() << "TestBtn text: " << testBtn->text();
			QVERIFY2(testBtn->text() == "Test command 300", "TestCommandMsg generated wrong results");
		},
		Qt::QueuedConnection);

	cmdQ->enqueue(cmd1);
	cmdQ->enqueue(cmd2);
	cmdQ->enqueue(cmd3);
	cmdQ->wait();
	delete cmdQ;
}

void TST_IioCommandQueue::testCommandOrder()
{
	CommandQueue *cmdQ = new CommandQueue(1, nullptr);
	Command *cmd1 = new TestCommandAdd(TEST_A, TEST_B, nullptr);

	Command *cmd3 = new TestCommandMsg(TEST_A, "Test command 300", nullptr);

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

			Command *cmd2 = new TestCommandMultiply(tcmd->getReturnCode(), TEST_B, nullptr);
			cmdQ->enqueue(cmd2);
			connect(
				cmd2, &scopy::Command::finished, this,
				[=](scopy::Command *cmd) {
					if(!cmd) {
						cmd = dynamic_cast<Command *>(QObject::sender());
					}
					TestCommandMultiply *tcmd = dynamic_cast<TestCommandMultiply *>(cmd);
					QVERIFY2(tcmd != nullptr, "Capture command is null");
					QVERIFY2(tcmd == cmd2, "Captured command not the expected one");
					QVERIFY2(tcmd->getResult() == "TEST OK",
						 "TestCommandMultiply not executed properly");
					QVERIFY2(tcmd->getReturnCode() == (TEST_B * (TEST_B + TEST_A)),
						 "TestCommandMultiply did not execute");
				},
				Qt::QueuedConnection);
		},
		Qt::QueuedConnection);

	connect(
		cmd3, &scopy::Command::finished, this,
		[=](scopy::Command *cmd) {
			if(!cmd) {
				cmd = dynamic_cast<Command *>(QObject::sender());
			}
			TestCommandMsg *tcmd = dynamic_cast<TestCommandMsg *>(cmd);
			QVERIFY2(tcmd != nullptr, "Capture command is null");
			QVERIFY2(tcmd == cmd3, "Captured command not the expected one");
			QVERIFY2(tcmd->getReturnCode() == TEST_A, "TestCommandMsg did not execute");
			QPushButton *testBtn = tcmd->getResult();
			QVERIFY2(testBtn != nullptr, "TestCommandMsg did not generate results");
			qDebug() << "TestBtn text: " << testBtn->text();
			QVERIFY2(testBtn->text() == "Test command 300", "TestCommandMsg generated wrong results");
		},
		Qt::QueuedConnection);

	cmdQ->enqueue(cmd1);
	cmdQ->enqueue(cmd3);
	cmdQ->wait();
	delete cmdQ;
}

/*
 * Creating the CommandQueue with just 1 possible running thread at a time
 * should allow us to enqueue commands from different threads but
 * the CommandQueue should ensure that the commands are executed in order,
 * one at a time, all on just one running thread.
 */
// void TST_IioCommandQueue::testLaunchCommandFromThread() {
//	CommandQueue *cmdQ = new CommandQueue(1, nullptr);
//	int m_nbOfThreads = 5;
//	m_commandExecThreadPool.setMaxThreadCount(m_nbOfThreads);

//	for (int i = 0; i < m_nbOfThreads * 2; i++) {
//		m_commandExecThreadPool.start([=] () {
//			Command *cmd = new TestCommandCounter(nullptr);
//			cmdQ->enqueue(cmd);
//		});
//	}

//	int waitResult = m_commandExecThreadPool.waitForDone();
//	qInfo() << "Thread pool wait result: " << waitResult;
//	qInfo() << "Command counter: " << TestCommandCounter::m_commandCounter;
//	qInfo() << "Thread pool expiry timeout: " << m_commandExecThreadPool.expiryTimeout();

//	QVERIFY2(TestCommandCounter::m_commandCounter == m_nbOfThreads * 2, "The threaded cmds were not executed in
// order/properly."); 	delete cmdQ;
//}

QTEST_MAIN(TST_IioCommandQueue)
#include "tst_iiocommandqueue.moc"
