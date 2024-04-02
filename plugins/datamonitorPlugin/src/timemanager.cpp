#include "timemanager.hpp"

#include <QApplication>
#include <QLoggingCategory>
#include <QwtDate>

Q_LOGGING_CATEGORY(CAT_TIME_TRACKER, "TimeTracker")

using namespace scopy;
using namespace datamonitor;

TimeManager *TimeManager::pinstance_{nullptr};

TimeManager::TimeManager(QObject *parent)
	: QObject(parent)
{

	m_startTime = QDateTime::currentDateTime();

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, [=, this]() {
		m_lastReadValue = QDateTime::currentDateTime();
		Q_EMIT timeout();
	});

	qDebug(CAT_TIME_TRACKER) << "ctor";
}

TimeManager::~TimeManager() { qDebug(CAT_TIME_TRACKER) << "dtor"; }

TimeManager *TimeManager::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new TimeManager(QApplication::instance()); // singleton has the app as parent
	} else {
		qDebug(CAT_TIME_TRACKER) << "got instance from singleton";
	}
	return pinstance_;
}

void TimeManager::setStartTime() { m_startTime = QDateTime::currentDateTime(); }

double TimeManager::startTime() const { return QwtDate::toDouble(m_startTime); }

QDateTime TimeManager::startTime() { return m_startTime; }

void TimeManager::startTimer() { m_timer->start(); }

void TimeManager::stopTimer() { m_timer->stop(); }

void TimeManager::setTimerInterval(double interval) { m_timer->setInterval(interval * 1000); }

bool TimeManager::isRunning() const { return m_isRunning; }

void TimeManager::setIsRunning(bool newIsRunning) { m_isRunning = newIsRunning; }

QDateTime TimeManager::lastReadValue() const { return m_lastReadValue; }
