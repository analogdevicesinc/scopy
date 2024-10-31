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

#include "messagebroker.h"

#include <QApplication>

using namespace scopy;

MessageBroker *MessageBroker::pinstance_{nullptr};

MessageBroker::MessageBroker(QObject *parent)
	: QObject(parent)
{}

MessageBroker::~MessageBroker() {}

MessageBroker *MessageBroker::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new MessageBroker(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

void MessageBroker::subscribe(QObject *obj, QString topic)
{
	if(!map.contains(obj))
		map.insert(obj, {"broadcast"});
	map[obj].insert(topic);
}

void MessageBroker::unsubscribe(QObject *obj, QString topic)
{
	if(map.contains(obj)) {
		map[obj].remove(topic);
		if(map[obj].count() == 0) {
			map.remove(obj);
		}
	}
}

void MessageBroker::publish(QString topic, QString message)
{

	bool processed;
	QList<QObject *> keys = map.keys();
	for(auto &&k : keys) {
		if(map.value(k).contains(topic)) {
			QMetaObject::invokeMethod(
				k, "messageCallback", Qt::AutoConnection,
				//						  Q_RETURN_ARG(bool, processed),
				Q_ARG(QString, topic), Q_ARG(QString, message));
		}
	}
}

#include "moc_messagebroker.cpp"
