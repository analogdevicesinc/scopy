#include <QTest>
#include <QVector>
#include <QPushButton>
#include <QSignalSpy>
#include <QString>
#include <QtConcurrent/QtConcurrent>

#include "iioutil/command.h"
#include "iioutil/commandqueue.h"

using namespace scopy;
Q_DECLARE_METATYPE(QSignalSpy*);
Q_DECLARE_METATYPE(CommandQueue*);


class TestCommand : public Command {
	Q_OBJECT
public:
	explicit TestCommand(int sleep, QObject *parent)
		: m_sleep(sleep) {
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual ~TestCommand() {
		qDebug() << "TestCommand deleted";
	}

	virtual void execute() override {
		Q_EMIT started(this);
		QThread::msleep(m_sleep);
		m_cmdResult->errorCode = m_sleep;
		Q_EMIT finished(this);
	}

private:
	int m_sleep;
};

class TestCommandResult : public Command {
	Q_OBJECT
public:
	explicit TestCommandResult(int sleep, std::string message, QObject *parent)
		: m_sleep(sleep)
		, m_msg(message) {
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual ~TestCommandResult() {
		qDebug() << "TestCommandResult deleted";
	}

	virtual void execute() override {
		Q_EMIT started(this);
		QThread::msleep(m_sleep);
		m_cmdResult->errorCode = m_sleep;
		m_cmdResult->results = (void*)m_msg.c_str();
		Q_EMIT finished(this);
	}

	std::string getResult()
	{
		return std::string((char*)m_cmdResult->results);
	}
private:
	int m_sleep;
	std::string m_msg;
};

class TestCommandResultObject : public Command {
	Q_OBJECT
public:
	explicit TestCommandResultObject(int sleep, QString message, QObject *parent)
		: m_sleep(sleep)
		, m_msg_btn(new QPushButton(message)) {
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual ~TestCommandResultObject() {
		qDebug() << "TestCommandResultObject deleted";
		if (m_cmdResult->results) {
			delete (QPushButton*)m_cmdResult->results;
			m_cmdResult->results = nullptr;
		}
	}

	virtual void execute() override {
		Q_EMIT started(this);
		QThread::msleep(m_sleep);
		m_cmdResult->errorCode = m_sleep;
		m_cmdResult->results = (void*)m_msg_btn;
		Q_EMIT finished(this);
	}

	QPushButton* getResult() {
		return static_cast<QPushButton*>(m_cmdResult->results);
	}

private:
	int m_sleep;
	QPushButton *m_msg_btn;
};

class TST_IioCommandQueue : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void testResults();
	void testSignalsCount();
	void testCommandsOrder();
	void testSameCommandMultipleTimes();
private:
	CommandQueue *cmdQ;
	Command *cmd1, *cmd2, *cmd3;
};

void TST_IioCommandQueue::testResults() {
	cmdQ = new CommandQueue(1, this);
	cmd1 = new TestCommand(100, cmdQ);
	cmd2 = new TestCommandResult(200, "Test command 200", cmdQ);
	cmd3 = new TestCommandResultObject(300, "Test command 300", cmdQ);

	QSignalSpy spy(cmd2, SIGNAL(finished(scopy::Command *)));

	connect(cmd1, &scopy::Command::finished,
		this, [=](scopy::Command* cmd) {
		if (!cmd) {
			cmd = dynamic_cast<Command*>(QObject::sender());
		}
		TestCommand *tcmd = dynamic_cast<TestCommand*>(cmd);
		QVERIFY2(tcmd != nullptr, "Capture command is null");
		QVERIFY2(tcmd == cmd1, "Captured command not the expected one");
		QVERIFY2(tcmd->getReturnCode() == 100, "TestCommand did not execute");
	}, Qt::QueuedConnection);

	connect(cmd2, &scopy::Command::finished,
		this, [=](scopy::Command* cmd) {
		if (!cmd) {
			cmd = dynamic_cast<Command*>(QObject::sender());
		}
		TestCommandResult *tcmd = dynamic_cast<TestCommandResult*>(cmd);
		QVERIFY2(tcmd != nullptr, "Capture command is null");
		QVERIFY2(tcmd == cmd2, "Captured command not the expected one");
		QVERIFY2(tcmd->getResult() == "Test command 200", "TestCommandResult did not execute correctly");
		QVERIFY2(tcmd->getReturnCode() == 200, "TestCommandResult did not execute");
	}, Qt::QueuedConnection);

	connect(cmd3, &scopy::Command::finished,
		this, [=](scopy::Command* cmd) {
		if (!cmd) {
			cmd = dynamic_cast<Command*>(QObject::sender());
		}
		TestCommandResultObject *tcmd = dynamic_cast<TestCommandResultObject*>(cmd);
		QVERIFY2(tcmd != nullptr, "Capture command is null");
		QVERIFY2(tcmd == cmd3, "Captured command not the expected one");
		QVERIFY2(tcmd->getReturnCode() == 300, "TestCommandResultObject did not execute");
		QPushButton *testBtn = tcmd->getResult();
		QVERIFY2(testBtn != nullptr, "TestCommandResultObject did not generate results");
		qDebug() << "TestBtn text: " << testBtn->text();
		QVERIFY2(testBtn->text() == "Test command 300", "TestCommandResultObject generated wrong results");
	}, Qt::QueuedConnection);

	cmdQ->enqueue(cmd3);
	cmdQ->enqueue(cmd1);
	cmdQ->enqueue(cmd2);
}

void TST_IioCommandQueue::testSignalsCount() {
	cmdQ = new CommandQueue(1, this);
	cmd1 = new TestCommand(100, cmdQ);
	cmd2 = new TestCommandResult(200, "Test command 200", cmdQ);
	cmd3 = new TestCommandResultObject(300, "Test command 300", cmdQ);

	QSignalSpy spy1(cmd1, SIGNAL(finished(scopy::Command *)));
	QSignalSpy spy2(cmd2, SIGNAL(finished(scopy::Command *)));
	QSignalSpy spy3(cmd3, SIGNAL(finished(scopy::Command *)));

	cmdQ->enqueue(cmd3);
	cmdQ->enqueue(cmd1);
	cmdQ->enqueue(cmd2);
	cmdQ->enqueue(cmd3);
//	cmdQ->requestStop();
//	cmdQ->wait();
//	QVERIFY2(spy1.count() == 1, "The first command did not emit 1 signal");
//	QVERIFY2(spy2.count() == 1, "The second command did not emit 1 signal");
//	QVERIFY2(spy3.count() == 2, "The third command did not emit 2 signals");
}

void TST_IioCommandQueue::testCommandsOrder() {
	cmdQ = new CommandQueue(1, this);
	cmd1 = new TestCommand(100, cmdQ);
	cmd2 = new TestCommandResult(200, "Test command 200", cmdQ);
	cmd3 = new TestCommandResultObject(300, "Test command 300", cmdQ);

	QSignalSpy spy(cmdQ, SIGNAL(finished(scopy::Command *)));

	cmdQ->enqueue(cmd3);
	cmdQ->enqueue(cmd1);
	cmdQ->enqueue(cmd2);
//	cmdQ->requestStop();
//	cmdQ->wait();
//	QVERIFY2(spy.count() == 3, "The command queue did not emit 3 signals");
//	QVERIFY2(qvariant_cast<TestCommandResultObject*>(spy.at(0).at(0)) == cmd3, "The 1st signal not emitted by TestCommandResultObject");
//	QVERIFY2(qvariant_cast<TestCommand*>(spy.at(1).at(0)) == cmd1, "The 2nd signal not emitted by TestCommand");
//	QVERIFY2(qvariant_cast<TestCommandResult*>(spy.at(2).at(0)) == cmd2, "The 3rd signal not emitted by TestCommandResult");
}

void TST_IioCommandQueue::testSameCommandMultipleTimes() {
	cmdQ = new CommandQueue(1, this);
	cmd1 = new TestCommand(100, cmdQ);

	QSignalSpy spy(cmdQ, SIGNAL(finished(scopy::Command *)));

	cmdQ->enqueue(cmd1);
	cmdQ->enqueue(cmd1);
	cmdQ->enqueue(cmd1);
	cmdQ->enqueue(cmd1);
//	cmdQ->requestStop();
//	cmdQ->wait();
//	QVERIFY2(spy.count() == 4, "The command queue did not emit 3 signals");
//	QVERIFY2(qvariant_cast<TestCommand*>(spy.at(0).at(0)) == cmd1, "The 1st signal not emitted by TestCommand");
//	QVERIFY2(qvariant_cast<TestCommand*>(spy.at(1).at(0)) == cmd1, "The 2nd signal not emitted by TestCommand");
//	QVERIFY2(qvariant_cast<TestCommand*>(spy.at(2).at(0)) == cmd1, "The 3rd signal not emitted by TestCommand");
//	QVERIFY2(qvariant_cast<TestCommand*>(spy.at(3).at(0)) == cmd1, "The 4th signal not emitted by TestCommand");
}


QTEST_MAIN(TST_IioCommandQueue)
#include "tst_iiocommandqueue.moc"


