#include "core/concurrentcmdexecutor.h"
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

class TestConcurrentCmdExecutor : public QObject
{
	Q_OBJECT
private slots:
	void parallelExecution();
	void cancelByResource();
	void pendingCount();
};

void TestConcurrentCmdExecutor::parallelExecution()
{
	ConcurrentCmdExecutor exec(4);
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

void TestConcurrentCmdExecutor::cancelByResource()
{
	int resource = 0;
	ConcurrentCmdExecutor exec(4);
	SlowCommand c1(100, &resource);
	SlowCommand c2(0, &resource);

	auto f1 = exec.execute(&c1);
	auto f2 = exec.execute(&c2);
	exec.cancelByResource(&resource);

	f1.waitForFinished();
	f2.waitForFinished();
}

void TestConcurrentCmdExecutor::pendingCount()
{
	ConcurrentCmdExecutor exec(4);
	QCOMPARE(exec.pendingCount(), 0);
}

QTEST_MAIN(TestConcurrentCmdExecutor)
#include "tst_concurrentcmdexecutor.moc"
