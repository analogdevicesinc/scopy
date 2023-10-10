#include "statusmanager.h"
#include <QApplication>
#include <QLoggingCategory>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_STATUSMANAGER, "StatusManager")

StatusManager *StatusManager::pinstance_{nullptr};

StatusManager::StatusManager(QObject *parent) {}

StatusManager::~StatusManager() {}

StatusManager *StatusManager::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new StatusManager(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

void StatusManager::addTemporaryMessage(const QString &message, int ms)
{
	m_itemQueue.append({QVariant(message), ms});
	if(m_itemQueue.size() > NO_RETAINED_MESSAGES) {
		m_itemQueue.pop_front();
	}

	Q_EMIT announceStatusAvailable();
}

void StatusManager::addTemporaryWidget(QWidget *widget, int ms)
{
	m_itemQueue.append({QVariant::fromValue(widget), ms});
	if(m_itemQueue.size() > NO_RETAINED_MESSAGES) {
		m_itemQueue.pop_front();
	}

	Q_EMIT announceStatusAvailable();
}

void StatusManager::requestStatus()
{
	if(!m_itemQueue.isEmpty()) {
		QPair<QVariant, int> res = m_itemQueue.first();
		m_itemQueue.pop_front();

		Q_EMIT sendStatus(res.first, res.second);
	}
}

#include "moc_statusmanager.cpp"
