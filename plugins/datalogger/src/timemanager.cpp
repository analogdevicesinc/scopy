/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

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

void TimeManager::setIsRunning(bool newIsRunning)
{
	m_isRunning = newIsRunning;
	Q_EMIT toggleRunning(newIsRunning);
}

QDateTime TimeManager::lastReadValue() const { return m_lastReadValue; }

#include "moc_timemanager.cpp"
