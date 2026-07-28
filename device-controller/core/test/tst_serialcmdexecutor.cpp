#include "core/pooledcmdexecutor.h"
#include "core/command.h"

#include <QSignalSpy>
#include <QTest>
#include <qelapsedtimer.h>

#include <cerrno>

using namespace scopy;

class DummyCommand : public Command
{
	Q_OBJECT
public:
	DummyCommand(int sleepMs = 0, void *resource = nullptr, QObject *parent = nullptr)
        : Command(resource, parent)
		, m_sleepMs(sleepMs)
    {
    }

	Result<void> result() const { return m_result; }
	int order() const { return m_order; }

	static void resetCounter() { s_counter = 0; }

protected:
    void run() override
    {
        if(m_sleepMs > 0) {
            QThread::msleep(m_sleepMs);
        }
        m_result = Result<void>{};
        m_order = s_counter++;
    }

private:
	int m_sleepMs;
	int m_order = -1;
	Result<void> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
	static inline int s_counter = 0;
};

class TestPooledSerialCmdExecutor : public QObject
{
	Q_OBJECT
private slots:
	void init() { DummyCommand::resetCounter(); }

	void serialOrder();
	void cancelByResource();
    void cancelById();
    void cancelAll();
	void pendingCount();
	void futureResolves();
};

void TestPooledSerialCmdExecutor::serialOrder()
{
    PooledCmdExecutor q;
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

void TestPooledSerialCmdExecutor::cancelByResource()
{
	int resource = 0;
    PooledCmdExecutor q;
	DummyCommand c1(50, &resource);
	DummyCommand c2(0, &resource);

	auto f1 = q.execute(&c1);
	auto f2 = q.execute(&c2);
	q.cancelByResource(&resource);

	f1.waitForFinished();
	f2.waitForFinished();

	QVERIFY(c2.isCancelled());
}

void TestPooledSerialCmdExecutor::cancelById()
{
    int resource = 0;
    PooledCmdExecutor q;
    DummyCommand c1(100, &resource);
    DummyCommand c2(200, &resource);

    auto f1 = q.execute(&c1);
    auto f2 = q.execute(&c2);

    q.cancelById(c2.id());

    f1.waitForFinished();
    QVERIFY(c2.isCancelled());
    QElapsedTimer timer;
    timer.start();
    f2.waitForFinished();
    QVERIFY(c2.isCancelled() && timer.elapsed() < 100);
}

void TestPooledSerialCmdExecutor::cancelAll()
{
    PooledCmdExecutor q;
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

void TestPooledSerialCmdExecutor::pendingCount()
{
    PooledCmdExecutor q;
	QCOMPARE(q.pendingCount(), 0);
}

void TestPooledSerialCmdExecutor::futureResolves()
{
    PooledCmdExecutor q;
	DummyCommand c;
	auto f = q.execute(&c);
	f.waitForFinished();
	QVERIFY(bool(c.result()));
}

QTEST_MAIN(TestPooledSerialCmdExecutor)
#include "tst_serialcmdexecutor.moc"
