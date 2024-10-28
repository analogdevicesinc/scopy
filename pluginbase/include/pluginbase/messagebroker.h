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

#ifndef MESSAGEBROKER_H
#define MESSAGEBROKER_H

#include "scopy-pluginbase_export.h"

#include <QMap>
#include <QObject>
#include <QSet>

namespace scopy {
/**
 * @brief The MessageBroker class
 * MessageBroker is a singleton class to implement a publisher-subscriber message exchange
 *
 */
class SCOPY_PLUGINBASE_EXPORT MessageBroker : public QObject
{
	Q_OBJECT
protected:
	MessageBroker(QObject *parent = nullptr);
	~MessageBroker();

public:
	// singleton
	MessageBroker(MessageBroker &other) = delete;
	void operator=(const MessageBroker &) = delete;
	static MessageBroker *GetInstance();

	/**
	 * @brief subscribe
	 * @param obj
	 * @param topic
	 * subscribe object to a topic
	 * when the topic is published, the object will receive the message
	 */
	void subscribe(QObject *obj, QString topic);

	/**
	 * @brief unsubscribe
	 * @param obj
	 * @param topic
	 * unsubscribe object from the topic
	 */
	void unsubscribe(QObject *obj, QString topic);

	/**
	 * @brief publish
	 * @param topic
	 * @param message
	 * publish a topic with a message
	 */
	void publish(QString topic, QString message);

private:
	QMap<QObject *, QSet<QString>> map;
	static MessageBroker *pinstance_;
};
} // namespace scopy

#endif // MESSAGEBROKER_H
