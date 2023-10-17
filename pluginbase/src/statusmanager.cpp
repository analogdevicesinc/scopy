#include "statusmanager.h"
#include <QApplication>
#include <QLoggingCategory>
#include <utility>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_STATUSMANAGER, "StatusManager")

StatusManager *StatusManager::pinstance_{nullptr};

StatusManager::StatusManager(QObject *parent)
	: m_timer(new QTimer(this))
	, m_itemQueue(new QList<StatusMessage *>)
	, m_enabled(false)
{
	connect(this, &StatusManager::messageAdded, this, &StatusManager::processStatusMessage);
	connect(m_timer, &QTimer::timeout, this, [this]() {
		m_timer->stop();
		Q_EMIT clearDisplay();
		processStatusMessage();
	});
}

StatusManager::~StatusManager() { delete m_itemQueue; }

StatusManager *StatusManager::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new StatusManager(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

void StatusManager::addTemporaryMessage(const QString &message, int ms)
{
	m_itemQueue->append(new StatusMessage(message, ms));
	if(m_itemQueue->size() > NUMBER_OF_RETAINED_MESSAGES) {
		m_itemQueue->pop_front();
	}

	if(m_enabled) {
		Q_EMIT messageAdded();
	}
}

void StatusManager::addTemporaryWidget(QWidget *widget, QString title, int ms)
{
	auto statusMessage = new StatusMessage(std::move(title), ms);
	statusMessage->setWidget(widget);
	if(ms == -1) {
		// permanent widget, the creator of this widget is responsible for calling delete on the widget
		connect(widget, &QWidget::destroyed, this, [this]() {
			m_timer->stop();
			clearDisplay();
			processStatusMessage();
		});
	}
	m_itemQueue->append(statusMessage);
	if(m_itemQueue->size() > NUMBER_OF_RETAINED_MESSAGES) {
		m_itemQueue->pop_front();
	}

	if(m_enabled) {
		Q_EMIT messageAdded();
	}
}

void StatusManager::addUrgentMessage(const QString &message, int ms)
{
	m_timer->stop();
	clearDisplay();
	auto statusMessage = new StatusMessage(message, ms);
	Q_EMIT sendStatus(statusMessage);
	m_timer->start(statusMessage->getDisplayTime());
}

void StatusManager::setEnabled(bool enabled) { m_enabled = enabled; }

bool StatusManager::isEnabled() const { return m_enabled; }

void StatusManager::processStatusMessage()
{
	if(!m_timer->isActive() && !m_itemQueue->isEmpty()) { // there is nothing displayed currently
		auto message = m_itemQueue->takeFirst();

		if(m_enabled) {
			Q_EMIT sendStatus(message);
			m_timer->start(message->getDisplayTime());
		}
	}
}

#include "moc_statusmanager.cpp"
