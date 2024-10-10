/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#include "statusbarmanager.h"
#include <QApplication>
#include <QLoggingCategory>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_STATUSBARMANAGER, "StatusBarManager")

StatusBarManager *StatusBarManager::pinstance_{nullptr};

StatusBarManager::StatusBarManager(QObject *parent)
	: m_timer(new QTimer(this))
	, m_itemQueue(new QList<StatusMessage *>)
	, m_enabled(false)
{
	connect(this, &StatusBarManager::messageAdded, this, &StatusBarManager::processStatusMessage);
	connect(m_timer, &QTimer::timeout, this, [this]() {
		m_timer->stop();
		Q_EMIT clearDisplay();
		processStatusMessage();
	});
}

StatusBarManager::~StatusBarManager() { delete m_itemQueue; }

StatusBarManager *StatusBarManager::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new StatusBarManager(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

void StatusBarManager::pushMessage(const QString &message, int ms)
{
	StatusBarManager::GetInstance()->_pushMessage(message, ms);
}

void StatusBarManager::pushWidget(QWidget *widget, QString title, int ms)
{
	StatusBarManager::GetInstance()->_pushWidget(widget, title, ms);
}

void StatusBarManager::pushUrgentMessage(const QString &message, int ms)
{
	StatusBarManager::GetInstance()->_pushUrgentMessage(message, ms);
}

void StatusBarManager::pushUrgentWidget(QWidget *widget, QString title, int ms)
{
	StatusBarManager::GetInstance()->_pushUrgentWidget(widget, title, ms);
}

void StatusBarManager::setEnabled(bool enabled) { m_enabled = enabled; }

bool StatusBarManager::isEnabled() const { return m_enabled; }

void StatusBarManager::processStatusMessage()
{
	if(!m_timer->isActive() && !m_itemQueue->isEmpty()) { // there is nothing displayed currently
		auto message = m_itemQueue->takeFirst();

		if(m_enabled) {
			qDebug(CAT_STATUSBARMANAGER) << "Displaying message:" << message->getText();
			Q_EMIT sendStatus(message);
			// must not block the timer for a permanent widget
			if(message->getDisplayTime() >= 0) {
				m_timer->start(message->getDisplayTime());
			}
		}
	}
}

void StatusBarManager::_pushMessage(const QString &message, int ms)
{
	m_itemQueue->append(new StatusMessageText(message, ms));
	if(m_itemQueue->size() > NUMBER_OF_RETAINED_MESSAGES) {
		m_itemQueue->pop_front();
	}

	if(m_enabled) {
		Q_EMIT messageAdded();
	}
}

void StatusBarManager::_pushWidget(QWidget *widget, QString title, int ms)
{
	auto statusMessage = new StatusMessageWidget(widget, title, ms);
	if(ms == INDEFINITE_DISPLAY_TIME) {
		// permanent widget, the creator of this widget is responsible for calling delete on the widget
		connect(widget, &QWidget::destroyed, this, [this]() {
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

void StatusBarManager::_pushUrgentMessage(const QString &message, int ms)
{
	if(m_timer->isActive()) {
		m_timer->stop();
		clearDisplay();
	}
	auto statusMessage = new StatusMessageText(message, ms);
	Q_EMIT sendStatus(statusMessage);
	m_timer->start(statusMessage->getDisplayTime());

	if(m_enabled) {
		Q_EMIT messageAdded();
	}
}

void StatusBarManager::_pushUrgentWidget(QWidget *widget, QString title, int ms)
{
	StatusMessageWidget *statusMessage = new StatusMessageWidget(widget, title, ms);
	if(ms == INDEFINITE_DISPLAY_TIME) {
		// permanent widget, the creator of this widget is responsible for calling delete on the widget
		connect(widget, &QWidget::destroyed, this, [this]() {
			clearDisplay();
			processStatusMessage();
		});
	}

	if(m_timer->isActive()) {
		m_timer->stop();
		clearDisplay();
	}

	Q_EMIT sendStatus(statusMessage);
	if(m_enabled) {
		Q_EMIT messageAdded();
	}
}

#include "moc_statusbarmanager.cpp"
