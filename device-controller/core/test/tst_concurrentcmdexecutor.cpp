#include "core/pooledcmdexecutor.h"
#include "core/command.h"

#include <QElapsedTimer>
#include <QTest>

#include <cerrno>

using namespace scopy;

class SlowCommand : public Command
{
	Q_OBJECT
public:
	SlowCommand(int sleepMs, void *resource = nullptr, QObject *parent = nullptr)
        : Command(resource, parent)
		, m_sleepMs(sleepMs)
    {
    }

	Result<void> result() const { return m_result; }

protected:
    void run() override
    {
        QThread::msleep(m_sleepMs);
        m_result = Result<void>{};
    }

private:
	int m_sleepMs;
	Result<void> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
};

class TestPooledCmdExecutor : public QObject
{
	Q_OBJECT
private slots:
	void parallelExecution();
	void cancelByResource();
    void cancelById();
	void pendingCount();
};

void TestPooledCmdExecutor::parallelExecution()
{
    PooledCmdExecutor exec(4);
	SlowCommand c1(100);
	SlowCommand c2(100);

	QElapsedTimer timer;
	timer.start();

	auto f1 = exec.execute(&c1);
	auto f2 = exec.execute(&c2);
	f1.waitForFinished();
	f2.waitForFinished();

	qint64 elapsed = timer.elapsed();
	QVERIFY(elapsed < 180);
	QVERIFY(bool(c1.result()));
	QVERIFY(bool(c2.result()));
}

void TestPooledCmdExecutor::cancelByResource()
{
	int resource = 0;
    PooledCmdExecutor exec(4);
	SlowCommand c1(100, &resource);
	SlowCommand c2(0, &resource);

	auto f1 = exec.execute(&c1);
	auto f2 = exec.execute(&c2);
	exec.cancelByResource(&resource);

	f2.waitForFinished();
}

void TestPooledCmdExecutor::cancelById()
{
    int resource = 0;
    PooledCmdExecutor exec(4);
    SlowCommand c1(100, &resource);
    SlowCommand c2(200, &resource);

    auto f1 = exec.execute(&c1);
    auto f2 = exec.execute(&c2);

    exec.cancelById(c2.id());

    QElapsedTimer timer;
    timer.start();

    f1.waitForFinished();

    QVERIFY(c2.isCancelled());
    f2.waitForFinished();
    QVERIFY(c2.isCancelled() && timer.elapsed() < 150);
}

void TestPooledCmdExecutor::pendingCount()
{
    PooledCmdExecutor exec(4);
	QCOMPARE(exec.pendingCount(), 0);
}

QTEST_MAIN(TestPooledCmdExecutor)
#include "tst_concurrentcmdexecutor.moc"
