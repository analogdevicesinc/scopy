#include "core/serialcmdexecutor.h"
#include "core/command.h"

#include <QSignalSpy>
#include <QTest>

#include <cerrno>

using namespace scopy;

class DummyCommand : public Command
{
	Q_OBJECT
public:
	DummyCommand(int sleepMs = 0, void *resource = nullptr, QObject *parent = nullptr)
		: Command(AttrRead, resource, parent)
		, m_sleepMs(sleepMs)
    {
    }

	void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			if(m_sleepMs > 0) {
				QThread::msleep(m_sleepMs);
			}
			m_result = Result<void>{};
			m_order = s_counter++;
		}
		Q_EMIT finished(this);
    }

	QString toString() const override { return QStringLiteral("DummyCommand"); }

	Result<void> result() const { return m_result; }
	int order() const { return m_order; }

	static void resetCounter() { s_counter = 0; }

private:
	int m_sleepMs;
	int m_order = -1;
	Result<void> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
	static inline int s_counter = 0;
};

class TestSerialCmdExecutor : public QObject
{
	Q_OBJECT
private slots:
	void init() { DummyCommand::resetCounter(); }

	void serialOrder();
	void cancelByResource();
	void cancelAll();
	void pendingCount();
	void futureResolves();
};

void TestSerialCmdExecutor::serialOrder()
{
	SerialCmdExecutor q;
	DummyCommand c1, c2, c3;
	auto f1 = q.execute(&c1);
	auto f2 = q.execute(&c2);
	auto f3 = q.execute(&c3);
	f1.waitForFinished();
	f2.waitForFinished();
	f3.waitForFinished();

	QVERIFY(c1.order() < c2.order());
	QVERIFY(c2.order() < c3.order());
}

void TestSerialCmdExecutor::cancelByResource()
{
	int resource = 0;
	SerialCmdExecutor q;
	DummyCommand c1(50, &resource);
	DummyCommand c2(0, &resource);

	auto f1 = q.execute(&c1);
	auto f2 = q.execute(&c2);
	q.cancelByResource(&resource);

	f1.waitForFinished();
	f2.waitForFinished();

	QVERIFY(c2.isCancelled());
}

void TestSerialCmdExecutor::cancelAll()
{
	SerialCmdExecutor q;
	DummyCommand c1(50);
	DummyCommand c2;
	DummyCommand c3;

	auto f1 = q.execute(&c1);
	auto f2 = q.execute(&c2);
	auto f3 = q.execute(&c3);
	q.cancelAll();

	f1.waitForFinished();
	f2.waitForFinished();
	f3.waitForFinished();
}

void TestSerialCmdExecutor::pendingCount()
{
	SerialCmdExecutor q;
	QCOMPARE(q.pendingCount(), 0);
}

void TestSerialCmdExecutor::futureResolves()
{
	SerialCmdExecutor q;
	DummyCommand c;
	auto f = q.execute(&c);
	f.waitForFinished();
	QVERIFY(bool(c.result()));
}

QTEST_MAIN(TestSerialCmdExecutor)
#include "tst_serialcmdexecutor.moc"
