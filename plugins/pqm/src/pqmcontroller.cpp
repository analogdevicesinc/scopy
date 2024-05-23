#include "pqmcontroller.h"
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(CAT_PQM_CONTROLLER, "PqmController")

using namespace scopy::pqm;

PqmController::PqmController(QString uri, QObject *parent)
	: QObject(parent)
	, m_uri(uri)
	, m_pingTask(nullptr)
	, m_pingTimer(nullptr)
{}

PqmController::~PqmController() { stopPingTask(); }

void PqmController::startPingTask(iio_context *ctx)
{
	if(!ctx) {
		qWarning(CAT_PQM_CONTROLLER) << "The context is unavailable!";
		return;
	}
	m_pingTask = new IIOPingTask(ctx);
	m_pingTimer = new CyclicalTask(m_pingTask);
	connect(m_pingTask, &IIOPingTask::pingFailed, this, &PqmController::pingFailed);
	connect(m_pingTask, &IIOPingTask::pingSuccess, this, &PqmController::pingSuccess);
	m_pingTimer->start(2000);
}

void PqmController::stopPingTask()
{
	if(m_pingTask && m_pingTimer) {
		m_pingTask->requestInterruption();
		m_pingTimer->deleteLater();
		m_pingTimer = nullptr;
		m_pingTask->deleteLater();
		m_pingTask = nullptr;
	}
}

#include "moc_pqmcontroller.cpp"
