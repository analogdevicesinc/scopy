#ifndef IIOCOMMANDQUEUE_H
#define IIOCOMMANDQUEUE_H

#include <deque>
#include <QThreadPool>
#include <QThread>

#include "command.h"

namespace scopy {
class SCOPY_IIOUTIL_EXPORT CommandQueue : public QObject
{
	Q_OBJECT
public:

	/**
	 * @brief IioCommandQueue::IioCommandQueue
	 * @param numberOfThreads
	 * @param parent
	 */
	explicit CommandQueue(int numberOfThreads = 1, QObject *parent = nullptr);
	~CommandQueue();
	void enqueue(Command *newCmd);
	int dequeue(Command *cmd);
	void start();
	void stop();
	void wait();
Q_SIGNALS:
	void started(scopy::Command *);
	void finished(scopy::Command *);
private Q_SLOTS:
	void work();
	void cmdStarted(scopy::Command *cmd);
	void cmdFinished(scopy::Command *cmd);
private:
	std::deque<Command*> m_commandQueue;
	std::mutex m_commandMutex;
	std::atomic<bool> m_running;
	QThreadPool m_commandExecThreadPool;
	QThread *m_workerThread;
	int m_nbThreads;
	bool m_async;
	bool m_workNewThread;
};
}
#endif // IIOCOMMANDQUEUE_H
