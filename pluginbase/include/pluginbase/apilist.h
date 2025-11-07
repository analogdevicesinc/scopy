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

#ifndef APILIST_H
#define APILIST_H

#include "scopy-pluginbase_export.h"
#include "apiobject.h"

#include <QMap>
#include <QPointer>
#include <QMutex>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(CAT_APILIST)

namespace scopy {

class SCOPY_PLUGINBASE_EXPORT ApiList
{
public:
	// Device-scoped API registration
	static void registerApi(const QString &deviceUri, const QString &apiName, ApiObject *api);

	// Device-scoped API unregistration
	static void unregisterApi(const QString &deviceUri, const QString &apiName);

	// Type-safe device-scoped API access with availability checking
	template <typename T>
	static T *getApi(const QString &deviceUri, const QString &apiName)
	{
		QMutexLocker locker(&s_mutex);
		auto deviceIt = s_deviceApis.find(deviceUri);
		if(deviceIt == s_deviceApis.end()) {
			return nullptr;
		}

		auto apiIt = deviceIt->find(apiName);
		if(apiIt == deviceIt->end() || apiIt->isNull()) {
			return nullptr;
		}
		return qobject_cast<T *>(apiIt->data());
	}

	// Check if API is available for specific device
	static bool isAvailable(const QString &deviceUri, const QString &apiName);

	// Get list of available APIs for specific device
	static QStringList availableApis(const QString &deviceUri);

	// Manual device cleanup
	static void removeDevice(const QString &deviceUri);

private:
	// Device URI -> (API Name -> API Object)
	static QMap<QString, QMap<QString, QPointer<ApiObject>>> s_deviceApis;
	static QMutex s_mutex;

	// Auto-cleanup when API object is destroyed
	static void onApiDestroyed();
};

} // namespace scopy

#endif // APILIST_H