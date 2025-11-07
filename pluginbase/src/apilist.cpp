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

#include "apilist.h"
#include <QDebug>

Q_LOGGING_CATEGORY(CAT_APILIST, "ApiList")

using namespace scopy;

// Static member definitions
QMap<QString, QPointer<ApiObject>> ApiList::s_apis;
QMutex ApiList::s_mutex;

void ApiList::registerApi(ApiObject *api)
{
	if(!api) {
		qWarning(CAT_APILIST) << "Cannot register null API";
		return;
	}

	QMutexLocker locker(&s_mutex);
	const QString objectName = api->objectName();

	if(s_apis.contains(objectName)) {
		qWarning(CAT_APILIST) << "API already registered for object:" << objectName << "- overriding";
	}

	s_apis[objectName] = QPointer<ApiObject>(api);
	qInfo(CAT_APILIST) << "Registered API:" << objectName;

	// Connect to destroyed signal for automatic cleanup
	QObject::connect(api, &QObject::destroyed, &ApiList::onApiDestroyed);
}

void ApiList::unregisterApi(ApiObject *api)
{
	if(!api) {
		return;
	}

	QMutexLocker locker(&s_mutex);
	const QString objectName = api->objectName();

	if(s_apis.remove(objectName) > 0) {
		qInfo(CAT_APILIST) << "Unregistered API:" << objectName;
	}
}

bool ApiList::isAvailable(const QString &objectName)
{
	QMutexLocker locker(&s_mutex);
	auto it = s_apis.find(objectName);
	return it != s_apis.end() && !it->isNull();
}

QStringList ApiList::availableApis()
{
	QMutexLocker locker(&s_mutex);
	QStringList available;
	for(auto it = s_apis.begin(); it != s_apis.end(); ++it) {
		if(!it->isNull()) {
			available << it.key();
		}
	}
	return available;
}

void ApiList::onApiDestroyed()
{
	QMutexLocker locker(&s_mutex);
	// Clean up null pointers from the map
	auto it = s_apis.begin();
	while(it != s_apis.end()) {
		if(it->isNull()) {
			qInfo(CAT_APILIST) << "Auto-cleaned destroyed API:" << it.key();
			it = s_apis.erase(it);
		} else {
			++it;
		}
	}
}