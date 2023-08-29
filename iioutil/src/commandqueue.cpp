#include "commandqueue.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#include <functional>

using namespace std;
using namespace scopy;

Q_LOGGING_CATEGORY(CAT_COMMANDQUEUE, "CommandQueue");

CommandQueue::CommandQueue(QObject *parent)
	: QObject(parent)
	, m_running(false)
{}

CommandQueue::~CommandQueue()
{
	requestStop();
	for(auto c : m_commandQueue) {
		delete c;
	}
	m_commandQueue.clear();
}

void CommandQueue::enqueue(Command *command)
{
	m_commandQueue.push_back(command);
	qDebug(CAT_COMMANDQUEUE) << "enqueued " << command << " " << m_commandQueue.size();

	if(!m_running) {
		start();
	}
}

void CommandQueue::start()
{
	m_running = true;
	runCmd();
}

void CommandQueue::resolveNext(scopy::Command *cmd)
{
	m_commandQueue.pop_front(); // also delete/disconnect
	qDebug(CAT_COMMANDQUEUE) << "delete " << cmd;
	disconnect(cmd, &Command::finished, this, &CommandQueue::resolveNext);
	cmd->deleteLater();

	if(m_commandQueue.size() == 0) {
		m_running = false;
	} else {
		runCmd();
	}
}

void CommandQueue::runCmd()
{
	std::lock_guard<std::mutex> lock(m_commandMutex);
	qDebug(CAT_COMMANDQUEUE) << "run cmd " << m_commandQueue.at(0);
	if(m_running) {
		connect(m_commandQueue.at(0), &Command::finished, this, &CommandQueue::resolveNext);
		QtConcurrent::run(QThreadPool::globalInstance(), std::bind([=]() {
					  std::unique_lock<std::mutex> lock(m_commandMutex);
					  qDebug(CAT_COMMANDQUEUE) << "execute start " << m_commandQueue.at(0);
					  m_commandQueue.at(0)->execute();
					  qDebug(CAT_COMMANDQUEUE) << "execute stop " << m_commandQueue.at(0);
				  }));
	}
}

void CommandQueue::requestStop()
{
	std::lock_guard<std::mutex> lock(m_commandMutex);
	qDebug(CAT_COMMANDQUEUE) << "request stop " << m_commandQueue.size();
	if(m_running) {
		m_running = false;
	}
}

void CommandQueue::wait() { QThreadPool::globalInstance()->waitForDone(); }

#include "moc_commandqueue.cpp"
