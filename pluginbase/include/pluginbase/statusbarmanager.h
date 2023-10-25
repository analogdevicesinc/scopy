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

#ifndef SCOPY_STATUSBARMANAGER_H
#define SCOPY_STATUSBARMANAGER_H

#include <QWidget>
#include <QVariant>
#include <QTimer>

#include "scopy-pluginbase_export.h"
#include "statusmessage.h"

#define NUMBER_OF_RETAINED_MESSAGES 10
#define DEFAULT_DISPLAY_TIME 5000

namespace scopy {
class SCOPY_PLUGINBASE_EXPORT StatusBarManager : public QObject
{
	Q_OBJECT
protected:
	StatusBarManager(QObject *parent = nullptr);
	~StatusBarManager();

public:
	// singleton
	StatusBarManager(StatusBarManager &other) = delete;
	void operator=(const StatusBarManager &) = delete;
	static StatusBarManager *GetInstance();

	/**
	 * @brief Ads temporary message in the queue to be displayed, when possible, in the Scopy status bar.
	 * @param message QString with the message to be displayed
	 * @param ms The time that the message will be displayed (in milliseconds)
	 * */
	static void pushMessage(const QString &message, int ms = DEFAULT_DISPLAY_TIME);

	/**
	 * @brief Ads temporary QWidget in the queue to be displayed, when possible, in the Scopy status bar. If there
	 * is no display time specified in the "ms" parameter, the creator of the widget is responsible for deleting
	 * it. This will enable StatusManager to send a signal that closes the display, ensuring a smooth transition.
	 * @param message QWidget* to be displayed
	 * @param title QString with the name of the message
	 * @param ms The time that the widget will be displayed (in milliseconds)
	 * */
	static void pushWidget(QWidget *widget, QString title, int ms = -1);

	/**
	 * @brief Overrides any message currently displayed with the message sent as parameter
	 * @param message QString with the urgent message
	 * @param ms The time that the message will be displayed (in milliseconds)
	 * */
	static void pushUrgentMessage(const QString &message, int ms = DEFAULT_DISPLAY_TIME);

	void setEnabled(bool enabled);
	bool isEnabled() const;

Q_SIGNALS:
	void sendStatus(StatusMessage *);

	void startDisplay(StatusMessage *);
	void clearDisplay();
	void messageAdded();

public Q_SLOTS:
	void processStatusMessage();

private:
	void _pushMessage(const QString &message, int ms);
	void _pushWidget(QWidget *widget, QString title, int ms);
	void _pushUrgentMessage(const QString &message, int ms);

	static StatusBarManager *pinstance_;
	QList<StatusMessage *> *m_itemQueue;
	QTimer *m_timer;
	bool m_enabled;
};
} // namespace scopy

#endif // SCOPY_STATUSBARMANAGER_H
