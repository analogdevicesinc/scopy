#include "commandqueue.h"

#include <functional>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

using namespace std;
using namespace scopy;

CommandQueue::CommandQueue(int numberOfThreads, QObject *parent)
	: QObject(parent)
	, m_running(false)
	, m_nbThreads(numberOfThreads)
	, m_async(m_nbThreads > 1)
	, m_workNewThread(m_nbThreads != 0)
	, m_workerThread(nullptr)
{
	m_commandExecThreadPool.setMaxThreadCount(std::min(m_nbThreads, QThread::idealThreadCount()));
	if (m_workNewThread) {
		m_workerThread = QThread::create(std::bind(&CommandQueue::work, this));
	}
}

CommandQueue::~CommandQueue()
{
	if (m_running) {
		m_running = false;
		if (m_workNewThread) {
			m_workerThread->requestInterruption();
			if (!m_workerThread->isFinished()) {
				m_workerThread->wait();
			}
			delete m_workerThread;
		}
	}

	std::unique_lock<std::mutex> lock(m_commandMutex);
	for (auto c : m_commandQueue) {
		delete c;
	}
	m_commandQueue.clear();
}

void CommandQueue::enqueue(Command *command)
{
	std::unique_lock<std::mutex> lock(m_commandMutex);
	start();
	connect(command, &Command::started, this, &CommandQueue::cmdStarted,
		static_cast<Qt::ConnectionType>((m_async ? Qt::QueuedConnection : Qt::DirectConnection) | Qt::UniqueConnection));
	connect(command, &Command::finished, this, &CommandQueue::cmdFinished,
		static_cast<Qt::ConnectionType>((m_async ? Qt::QueuedConnection : Qt::DirectConnection) | Qt::UniqueConnection));
	m_commandQueue.push_back(command);
}

int CommandQueue::dequeue(Command *cmd)
{
	std::unique_lock<std::mutex> lock(m_commandMutex);
	int occurenceCount = 0;
	auto found = std::find(m_commandQueue.begin(), m_commandQueue.end(), cmd);
	while (found != m_commandQueue.end()) {
		occurenceCount++;
		auto it = m_commandQueue.erase(found);
		disconnect(*it, &Command::finished, this, &CommandQueue::cmdFinished);
		disconnect(*it, &Command::started, this, &CommandQueue::cmdStarted);
		found = std::find(m_commandQueue.begin(), m_commandQueue.end(), cmd);
	}
	return occurenceCount;
}

void CommandQueue::start()
{
	if (!m_running) {
		m_running = true;
		if (m_workNewThread) {
			if (!m_workerThread->isRunning()) {
				m_workerThread->start();
			}
		} else {
			 // trigger work on Main Thread
			QMetaObject::invokeMethod(this, "work", Qt::QueuedConnection);
		}
	}
}

void CommandQueue::stop()
{
	if (m_running) {
		m_running = false;
		if (m_workNewThread) {
			m_workerThread->requestInterruption();
		}
	}
}

void CommandQueue::wait()
{
	if (m_running && !m_workerThread) {
		m_running = false;
	}
	if (m_workNewThread) {
		if (!m_workerThread->isFinished()) {
			m_workerThread->wait();
		}
	}
}

void CommandQueue::work()
{
	while (m_running) {
		std::unique_lock<std::mutex> lock(m_commandMutex);
		if (m_commandQueue.size() == 0) {
			if (!m_workNewThread) {
				m_running = false;
			}
			lock.unlock();
			break;
		}
		Command *currentCmd = m_commandQueue.front();
		if (m_async) {
			// TBD: should we keep a vector of futures to wait on them?
			QtConcurrent::run(&m_commandExecThreadPool, std::bind([=]() {
				currentCmd->execute();
			}));
		} else {
			currentCmd->execute();
		}
		m_commandQueue.pop_front();
		lock.unlock();
	}
}

void CommandQueue::cmdFinished(scopy::Command *cmd)
{
	// QObject::sender() does not work if signal is emitted from another
	// thread and DirectConnection is used.
	if (!cmd) {
		cmd = dynamic_cast<Command*>(QObject::sender());
	}
	if (cmd) {
		Q_EMIT finished(cmd);
	}
}

void CommandQueue::cmdStarted(scopy::Command *cmd)
{
	if (!cmd) {
		cmd = dynamic_cast<Command*>(QObject::sender());
	}
	if (cmd) {
		Q_EMIT started(cmd);
	}
}
