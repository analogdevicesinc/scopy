#include "commandqueue.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#include <functional>

using namespace std;
using namespace scopy;

Q_LOGGING_CATEGORY(CAT_COMMANDQUEUE, "CommandQueue");

CommandQueue::CommandQueue(int numberOfThreads, QObject *parent)
	: QObject(parent)
	, m_running(false)
	, m_nbThreads(numberOfThreads)
	, m_async(m_nbThreads > 1)
	, m_workNewThread(m_nbThreads != 0)
	, m_currentCommand(nullptr)
{
	m_commandExecThreadPool.setMaxThreadCount(std::min(m_nbThreads, QThread::idealThreadCount()));
}

CommandQueue::~CommandQueue()
{
	requestStop();
	wait();

	for(auto c : m_commandQueue) {
		delete c;
	}
	m_commandQueue.clear();
}

void CommandQueue::enqueue(Command *command)
{
	std::lock_guard<std::mutex> lock(m_commandMutex);
	start();
	connect(command, &Command::started, this, &CommandQueue::cmdStarted,
		static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
	connect(command, &Command::finished, this, &CommandQueue::cmdFinished,
		static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
	m_commandQueue.push_back(command);
}

void CommandQueue::start()
{
	if(!m_running) {
		qDebug(CAT_COMMANDQUEUE) << "CommandQueue set running to true (start)";
		m_running = true;
		if(m_workNewThread) {
			m_commandExecThreadPool.start(std::bind(&CommandQueue::work, this));
		} else {
			// trigger work on Main Thread
			QMetaObject::invokeMethod(this, "work", Qt::QueuedConnection);
		}
	}
}

void CommandQueue::requestStop()
{
	if(m_running) {
		qDebug(CAT_COMMANDQUEUE) << "CommandQueue set running to false (stop)";
		m_running = false;
		if(m_workNewThread) {
			std::unique_lock<std::mutex> lock(m_commandMutex);
			m_commandQueue.clear();
		}
	}
}

void CommandQueue::wait()
{
	if(m_running) {
		qDebug(CAT_COMMANDQUEUE) << "CommandQueue set running to true (wait)";
		m_running = false;
	}
	if(m_workNewThread) {
		m_commandExecThreadPool.waitForDone();
	}
}

void CommandQueue::work()
{
	while(m_running) {
		std::unique_lock<std::mutex> lock(m_commandMutex);
		if(m_commandQueue.empty()) {
			m_running = false;
			break;
		}

		m_currentCommand = m_commandQueue.front();
		if(m_async) {
			QtConcurrent::run(&m_commandExecThreadPool, std::bind([=]() { m_currentCommand->execute(); }));
		} else {
			int size = m_commandQueue.size();
			lock.unlock();
			qDebug(CAT_COMMANDQUEUE)
				<< "CommandQueue executing " << m_currentCommand << " cmdq size: " << size;
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
	if(!cmd) {
		cmd = dynamic_cast<Command *>(QObject::sender());
	}
	if(cmd) {
		Q_EMIT finished(cmd);
	}
}

void CommandQueue::cmdStarted(scopy::Command *cmd)
{
	if(!cmd) {
		cmd = dynamic_cast<Command *>(QObject::sender());
	}
	if(cmd) {
		Q_EMIT started(cmd);
	}
}

#include "moc_commandqueue.cpp"
