#include "timetracker.hpp"

#include <QApplication>
#include <QDateTime>
#include <QLoggingCategory>
#include <QwtDate>

Q_LOGGING_CATEGORY(CAT_TIME_TRACKER, "TimeTracker")

using namespace scopy;
using namespace datamonitor;

TimeTracker *TimeTracker::pinstance_{nullptr};

TimeTracker::TimeTracker(QObject *parent)
	: QObject(parent)
{
	qDebug(CAT_TIME_TRACKER) << "ctor";
}

TimeTracker::~TimeTracker() { qDebug(CAT_TIME_TRACKER) << "dtor"; }

TimeTracker *TimeTracker::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new TimeTracker(QApplication::instance()); // singleton has the app as parent
	} else {
		qDebug(CAT_TIME_TRACKER) << "got instance from singleton";
	}
	return pinstance_;
}

void TimeTracker::setStartTime() { m_startTime = QDateTime::currentMSecsSinceEpoch(); }

double TimeTracker::time()
{
	if(m_realTime) {
		return QwtDate::toDouble(QDateTime::currentDateTime());
	}
	return (QDateTime::currentMSecsSinceEpoch() - m_startTime) / 1000;
}

void TimeTracker::toggleRealTime(bool toggled) { m_realTime = toggled; }

double TimeTracker::startTime() const { return m_startTime; }
