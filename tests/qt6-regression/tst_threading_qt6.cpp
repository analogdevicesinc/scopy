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
#include <QThread>
#include <QTimer>
#include <QEventLoop>
#include <QSignalSpy>
#include <atomic>

class MockThread : public QThread
{
	Q_OBJECT
public:
	MockThread(int sleepMs = 50, QObject *parent = nullptr)
		: QThread(parent)
		, m_sleepMs(sleepMs)
	{}

	std::atomic<int> runCount{0};

protected:
	void run() override
	{
		runCount++;
		QThread::msleep(m_sleepMs);
	}

private:
	int m_sleepMs;
};

class TST_ThreadingQt6 : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void thread_start_stop();
	void thread_isFinished();
	void thread_restart();
	void thread_doubleStart();
	void timer_signal_delivery();
	void timer_periodic_restart();
	void thread_wait_timeout();
};

void TST_ThreadingQt6::thread_start_stop()
{
	MockThread thread(50);

	thread.start();
	QVERIFY(thread.isRunning() || thread.isFinished());
	QVERIFY(thread.wait(5000));
	QVERIFY(thread.isFinished());
	QCOMPARE(thread.runCount.load(), 1);
}

void TST_ThreadingQt6::thread_isFinished()
{
	MockThread thread(50);

	QVERIFY(!thread.isRunning());
	QVERIFY(!thread.isFinished());

	thread.start();
	QVERIFY(thread.wait(5000));

	QVERIFY(thread.isFinished());
	QVERIFY(!thread.isRunning());
}

void TST_ThreadingQt6::thread_restart()
{
	MockThread thread(20);

	thread.start();
	QVERIFY(thread.wait(5000));
	QVERIFY(thread.isFinished());
	QCOMPARE(thread.runCount.load(), 1);

	thread.start();
	QVERIFY(thread.wait(5000));
	QVERIFY(thread.isFinished());
	QCOMPARE(thread.runCount.load(), 2);
}

void TST_ThreadingQt6::thread_doubleStart()
{
	MockThread thread(100);

	thread.start();
	thread.start();

	QVERIFY(thread.wait(5000));
	QVERIFY(thread.isFinished());
	QCOMPARE(thread.runCount.load(), 1);
}

void TST_ThreadingQt6::timer_signal_delivery()
{
	QTimer timer;
	int fireCount = 0;

	connect(&timer, &QTimer::timeout, this, [&fireCount]() { fireCount++; });

	timer.start(50);

	QEventLoop loop;
	QTimer::singleShot(300, &loop, &QEventLoop::quit);
	loop.exec();

	timer.stop();

	QVERIFY2(fireCount >= 3, qPrintable(QString("Expected at least 3 fires, got %1").arg(fireCount)));
	QVERIFY2(fireCount <= 10, qPrintable(QString("Expected at most 10 fires, got %1").arg(fireCount)));
}

void TST_ThreadingQt6::timer_periodic_restart()
{
	MockThread *thread = new MockThread(20, this);
	QTimer timer;
	int restartCount = 0;

	connect(&timer, &QTimer::timeout, this, [thread, &restartCount]() {
		if(thread->isFinished()) {
			thread->start();
			restartCount++;
		}
	});

	thread->start();
	timer.start(100);

	QEventLoop loop;
	QTimer::singleShot(500, &loop, &QEventLoop::quit);
	loop.exec();

	timer.stop();
	thread->wait(5000);

	QVERIFY2(restartCount >= 2, qPrintable(QString("Expected at least 2 restarts, got %1").arg(restartCount)));
	QVERIFY2(thread->runCount.load() >= 3,
		 qPrintable(QString("Expected at least 3 total runs, got %1").arg(thread->runCount.load())));
}

void TST_ThreadingQt6::thread_wait_timeout()
{
	MockThread thread(500);

	thread.start();

	bool finished = thread.wait(50);
	QVERIFY(!finished);

	QVERIFY(thread.wait(5000));
	QVERIFY(thread.isFinished());
}

QTEST_MAIN(TST_ThreadingQt6)
#include "tst_threading_qt6.moc"
