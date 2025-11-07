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
QMap<QString, QMap<QString, QPointer<ApiObject>>> ApiList::s_deviceApis;
QMutex ApiList::s_mutex;

void ApiList::registerApi(const QString &deviceUri, const QString &apiName, ApiObject *api)
{
	if(!api) {
		qWarning(CAT_APILIST) << "Cannot register null API";
		return;
	}

	QMutexLocker locker(&s_mutex);

	if(s_deviceApis[deviceUri].contains(apiName)) {
		qWarning(CAT_APILIST) << "API already registered for device:" << deviceUri << "API:" << apiName
				      << "- overriding";
	}

	s_deviceApis[deviceUri][apiName] = QPointer<ApiObject>(api);
	qInfo(CAT_APILIST) << "Registered API:" << apiName << "for device:" << deviceUri;

	// Connect to destroyed signal for automatic cleanup
	QObject::connect(api, &QObject::destroyed, &ApiList::onApiDestroyed);
}

void ApiList::unregisterApi(const QString &deviceUri, const QString &apiName)
{
	QMutexLocker locker(&s_mutex);

	auto deviceIt = s_deviceApis.find(deviceUri);
	if(deviceIt != s_deviceApis.end()) {
		if(deviceIt->remove(apiName) > 0) {
			qInfo(CAT_APILIST) << "Unregistered API:" << apiName << "for device:" << deviceUri;
		}
	}
}

bool ApiList::isAvailable(const QString &deviceUri, const QString &apiName)
{
	QMutexLocker locker(&s_mutex);
	auto deviceIt = s_deviceApis.find(deviceUri);
	if(deviceIt == s_deviceApis.end()) {
		return false;
	}
	auto apiIt = deviceIt->find(apiName);
	return apiIt != deviceIt->end() && !apiIt->isNull();
}

QStringList ApiList::availableApis(const QString &deviceUri)
{
	QMutexLocker locker(&s_mutex);
	QStringList available;
	auto deviceIt = s_deviceApis.find(deviceUri);
	if(deviceIt != s_deviceApis.end()) {
		for(auto it = deviceIt->begin(); it != deviceIt->end(); ++it) {
			if(!it->isNull()) {
				available << it.key();
			}
		}
	}
	return available;
}

void ApiList::removeDevice(const QString &deviceUri)
{
	QMutexLocker locker(&s_mutex);
	int removedApis = s_deviceApis.remove(deviceUri);
	if(removedApis > 0) {
		qInfo(CAT_APILIST) << "Removed device and all APIs:" << deviceUri;
	}
}

void ApiList::onApiDestroyed()
{
	QMutexLocker locker(&s_mutex);
	// Clean up null pointers from all device maps
	for(auto deviceIt = s_deviceApis.begin(); deviceIt != s_deviceApis.end(); ++deviceIt) {
		auto apiIt = deviceIt->begin();
		while(apiIt != deviceIt->end()) {
			if(apiIt->isNull()) {
				qInfo(CAT_APILIST) << "Auto-cleaned destroyed API:" << apiIt.key()
						   << "for device:" << deviceIt.key();
				apiIt = deviceIt->erase(apiIt);
			} else {
				++apiIt;
			}
		}
	}
}