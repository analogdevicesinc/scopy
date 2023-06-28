#include "commandqueue.h"

#include <functional>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

using namespace std;
using namespace scopy;

Q_LOGGING_CATEGORY(CAT_COMMANDQUEUE, "CommandQueue");

CommandQueue::CommandQueue(int numberOfThreads, QObject *parent)
	: QObject(parent)
	, m_running(false)
	, m_nbThreads(numberOfThreads)
	, m_async(m_nbThreads > 1)
	, m_workNewThread(m_nbThreads != 0)
	, m_workerThread(nullptr)
	, m_currentCommand(nullptr)
{
	m_commandExecThreadPool.setMaxThreadCount(std::min(m_nbThreads, QThread::idealThreadCount()));
	if (m_workNewThread) {
		m_workerThread = QThread::create(std::bind(&CommandQueue::work, this));
		m_workerThread->setParent(this);
	}
}

CommandQueue::~CommandQueue()
{
	requestStop();
	m_cond.notify_one();
	wait();
	m_workerThread->deleteLater();

	for (auto c : m_commandQueue) {
		delete c;
	}
	m_commandQueue.clear();
}

void CommandQueue::enqueue(Command *command)
{
	std::lock_guard<std::mutex> lock(m_commandMutex);
	if (command->isOverwrite()) {
		int occurences = removeUnlocked(command);
		qDebug(CAT_COMMANDQUEUE) << "Overwritten occurences: " << occurences;
	}
	start();
	connect(command, &Command::started, this, &CommandQueue::cmdStarted,
		static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
	connect(command, &Command::finished, this, &CommandQueue::cmdFinished,
		static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
	m_commandQueue.push_back(command);
	m_cond.notify_one();
}

int CommandQueue::remove(Command *cmd)
{
	std::lock_guard<std::mutex> lock(m_commandMutex);
	int occurences = removeUnlocked(cmd);
	m_cond.notify_one();
	return occurences;
}

int CommandQueue::removeUnlocked(Command *cmd)
{
	int occurenceCount = 0;
	// Skip the front element when searching, that one might be in execution
	auto found = std::find(m_commandQueue.begin(), m_commandQueue.end(), cmd);
	while (found != m_commandQueue.end()) {
		if (*found != m_currentCommand) {
			occurenceCount++;
			auto it = m_commandQueue.erase(found);
			disconnect(*it, &Command::finished, this, &CommandQueue::cmdFinished);
			disconnect(*it, &Command::started, this, &CommandQueue::cmdStarted);
			found = std::find(m_commandQueue.begin(), m_commandQueue.end(), cmd);
		}
	}
	return occurenceCount;
}

void CommandQueue::start()
{
	if (!m_running) {
		qDebug(CAT_COMMANDQUEUE) << "CommandQueue set running to true (start)";
		m_running = true;
		if (m_workNewThread) {
			if (!m_workerThread->isRunning()) {
				m_workerThread->start();
				qDebug(CAT_COMMANDQUEUE) << "CommandQueue START";
			}
		} else {
			 // trigger work on Main Thread
			QMetaObject::invokeMethod(this, "work", Qt::QueuedConnection);
		}
	}
}

void CommandQueue::requestStop()
{
	if (m_running) {
		qDebug(CAT_COMMANDQUEUE) << "CommandQueue set running to false (stop)";
		m_running = false;
		if (m_workNewThread) {
			if (m_workerThread->isRunning()) {
				m_workerThread->requestInterruption();
			}
		}
	}
}

void CommandQueue::wait()
{
	if (m_running && !m_workerThread) {
		qDebug(CAT_COMMANDQUEUE) << "CommandQueue set running to true (wait)";
		m_running = false;
	}
	if (m_workNewThread) {
		if (!m_workerThread->isFinished() && m_workerThread->isInterruptionRequested()) {
			m_workerThread->wait();
		}
	}
}

void CommandQueue::work()
{
	while (m_running) {
		std::unique_lock<std::mutex> lock(m_commandMutex);
		if (m_workNewThread) {
			m_cond.wait(lock, [=] {
				return !m_commandQueue.empty() || m_workerThread->isInterruptionRequested();
			});
			if (m_workerThread->isInterruptionRequested()) {
				m_commandQueue.clear();
				break;
			}
		} else if (m_commandQueue.empty()) {
			m_running = false;
			break;
		}

		m_currentCommand = m_commandQueue.front();
		if (m_async) {
			QtConcurrent::run(&m_commandExecThreadPool, std::bind([=]() {
				m_currentCommand->execute();
			}));
		} else {
			lock.unlock();
			qDebug(CAT_COMMANDQUEUE) << "CommandQueue executing " << m_currentCommand;
			m_currentCommand->execute();
			m_currentCommand->deleteLater();
			m_currentCommand = nullptr;
			lock.lock();
		}
		m_commandQueue.pop_front();
	}
}

void CommandQueue::cmdFinished(scopy::Command *cmd)
{
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
