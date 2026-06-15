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
#include <QtConcurrent>

class SignalEmitter : public QObject
{
	Q_OBJECT
public:
	using QObject::QObject;

Q_SIGNALS:
	void signalA(int value);
	void signalB(int value);

public Q_SLOTS:
	void emitSignalA(int value) { Q_EMIT signalA(value); }
	void emitSignalB(int value) { Q_EMIT signalB(value); }
};

class WorkerThread : public QThread
{
	Q_OBJECT
public:
	WorkerThread(int value, QObject *parent = nullptr)
		: QThread(parent)
		, m_value(value)
	{}

Q_SIGNALS:
	void workDone(int result);

protected:
	void run() override
	{
		QThread::msleep(10);
		Q_EMIT workDone(m_value * 2);
	}

private:
	int m_value;
};

class TST_SignalSlotQt6 : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void directConnection_lambda();
	void queuedConnection_crossThread();
	void queuedConnection_lambdaCapture();
	void autoConnection_sameThread();
	void autoConnection_crossThread();
	void signalChaining();
	void disconnectDuringSigEmit();
};

void TST_SignalSlotQt6::directConnection_lambda()
{
	SignalEmitter emitter;
	int received = 0;

	connect(
		&emitter, &SignalEmitter::signalA, this, [&received](int value) { received = value; },
		Qt::DirectConnection);

	Q_EMIT emitter.signalA(42);
	QCOMPARE(received, 42);
}

void TST_SignalSlotQt6::queuedConnection_crossThread()
{
	WorkerThread worker(21);
	int result = 0;

	connect(
		&worker, &WorkerThread::workDone, this, [&result](int value) { result = value; }, Qt::QueuedConnection);

	worker.start();
	QVERIFY(worker.wait(5000));

	QCoreApplication::processEvents();
	QCOMPARE(result, 42);
}

void TST_SignalSlotQt6::queuedConnection_lambdaCapture()
{
	WorkerThread worker(10);
	int capturedValue = 99;
	int result = 0;

	connect(
		&worker, &WorkerThread::workDone, this,
		[capturedValue, &result](int value) { result = value + capturedValue; }, Qt::QueuedConnection);

	worker.start();
	QVERIFY(worker.wait(5000));

	QCoreApplication::processEvents();
	QCOMPARE(result, 20 + 99);
}

void TST_SignalSlotQt6::autoConnection_sameThread()
{
	SignalEmitter emitter;
	int received = 0;

	connect(&emitter, &SignalEmitter::signalA, this, [&received](int value) { received = value; });

	Q_EMIT emitter.signalA(77);
	QCOMPARE(received, 77);
}

void TST_SignalSlotQt6::autoConnection_crossThread()
{
	WorkerThread worker(5);
	int result = 0;

	connect(&worker, &WorkerThread::workDone, this, [&result](int value) { result = value; });

	worker.start();
	QVERIFY(worker.wait(5000));

	QCoreApplication::processEvents();
	QCOMPARE(result, 10);
}

void TST_SignalSlotQt6::signalChaining()
{
	SignalEmitter emitterA;
	SignalEmitter emitterB;
	int finalResult = 0;

	connect(&emitterA, &SignalEmitter::signalA, &emitterB, &SignalEmitter::emitSignalB);
	connect(&emitterB, &SignalEmitter::signalB, this, [&finalResult](int value) { finalResult = value; });

	Q_EMIT emitterA.signalA(33);
	QCOMPARE(finalResult, 33);
}

void TST_SignalSlotQt6::disconnectDuringSigEmit()
{
	SignalEmitter emitter;
	int callCount = 0;
	QMetaObject::Connection conn;

	conn = connect(&emitter, &SignalEmitter::signalA, this, [&callCount, &conn, &emitter](int) {
		callCount++;
		disconnect(conn);
	});

	Q_EMIT emitter.signalA(1);
	Q_EMIT emitter.signalA(2);

	QCOMPARE(callCount, 1);
}

QTEST_MAIN(TST_SignalSlotQt6)
#include "tst_signalslot_qt6.moc"
