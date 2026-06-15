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
 */

#include <QTest>
#include <QSignalSpy>
#include <QTime>
#include <atomic>

#include <iioutil/command.h>
#include <iioutil/commandqueue.h>

using namespace scopy;

class TestCommandCounter : public Command
{
	Q_OBJECT
public:
	TestCommandCounter(QObject *parent = nullptr)
		: Command()
	{
		m_cmdResult = new CommandResult();
	}

	void execute() override
	{
		Q_EMIT started(this);
		s_counter++;
		m_cmdResult->errorCode = s_counter.load();
		Q_EMIT finished(this);
	}

	static std::atomic<int> s_counter;
};

std::atomic<int> TestCommandCounter::s_counter{0};

class TestCommandSleep : public Command
{
	Q_OBJECT
public:
	TestCommandSleep(int sleepMs, QObject *parent = nullptr)
		: Command()
		, m_sleepMs(sleepMs)
	{
		m_cmdResult = new CommandResult();
	}

	void execute() override
	{
		Q_EMIT started(this);
		QThread::msleep(m_sleepMs);
		m_cmdResult->errorCode = m_sleepMs;
		Q_EMIT finished(this);
	}

private:
	int m_sleepMs;
};

class TestCommandAdd : public Command
{
	Q_OBJECT
public:
	TestCommandAdd(int a, int b, QObject *parent = nullptr)
		: Command()
		, m_a(a)
		, m_b(b)
	{
		m_cmdResult = new CommandResult();
	}

	void execute() override
	{
		Q_EMIT started(this);
		m_cmdResult->errorCode = m_a + m_b;
		Q_EMIT finished(this);
	}

private:
	int m_a;
	int m_b;
};

class TST_CommandQueueQt6 : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void init();
	void qtconcurrent_run_executes();
	void command_finished_signal();
	void qtime_lastCmdTime();
};

void TST_CommandQueueQt6::init() { TestCommandCounter::s_counter = 0; }

void TST_CommandQueueQt6::qtconcurrent_run_executes()
{
	CommandQueue *cmdQ = new CommandQueue(this);

	cmdQ->enqueue(new TestCommandCounter());
	cmdQ->enqueue(new TestCommandCounter());
	cmdQ->enqueue(new TestCommandCounter());

	QTRY_COMPARE_WITH_TIMEOUT(TestCommandCounter::s_counter.load(), 3, 5000);

	delete cmdQ;
}

void TST_CommandQueueQt6::command_finished_signal()
{
	CommandQueue *cmdQ = new CommandQueue(this);
	auto *cmd = new TestCommandAdd(7, 3);
	int receivedResult = 0;

	connect(
		cmd, &Command::finished, this, [&receivedResult](Command *c) { receivedResult = c->getReturnCode(); },
		Qt::QueuedConnection);

	cmdQ->enqueue(cmd);

	QTRY_COMPARE_WITH_TIMEOUT(receivedResult, 10, 5000);

	delete cmdQ;
}

void TST_CommandQueueQt6::qtime_lastCmdTime()
{
	CommandQueue *cmdQ = new CommandQueue(this);

	cmdQ->enqueue(new TestCommandSleep(50));

	QTRY_VERIFY_WITH_TIMEOUT(TestCommandCounter::s_counter.load() >= 0, 5000);
	QTest::qWait(200);

	QTime lastTime = cmdQ->lastCmdTime();
	QTime now = QTime::currentTime();
	int elapsed = lastTime.msecsTo(now);

	QVERIFY2(elapsed >= 0, "lastCmdTime should be in the past");
	QVERIFY2(elapsed < 2000, "lastCmdTime should be recent (within 2s)");

	delete cmdQ;
}

QTEST_MAIN(TST_CommandQueueQt6)
#include "tst_commandqueue_qt6.moc"
