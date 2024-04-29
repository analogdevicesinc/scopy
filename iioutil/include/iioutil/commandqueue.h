#ifndef IIOCOMMANDQUEUE_H
#define IIOCOMMANDQUEUE_H

#include "command.h"

#include <QThread>
#include <QThreadPool>

#include <deque>

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
	explicit CommandQueue(QObject *parent = nullptr);
	~CommandQueue();
	void enqueue(Command *newCmd);
	void start();
	void wait();
	void requestStop();
	void runCmd();
private Q_SLOTS:
	void resolveNext(scopy::Command *cmd);

private:
	std::deque<Command *> m_commandQueue;
	std::mutex m_commandMutex;
	std::mutex m_enqueueMutex;
	std::atomic<bool> m_running;
	QThreadPool m_commandExecThreadPool;
};
} // namespace scopy
#endif // IIOCOMMANDQUEUE_H
