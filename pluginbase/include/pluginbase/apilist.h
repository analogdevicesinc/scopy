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
	// Register API (similar to ScopyJS::registerApi)
	static void registerApi(ApiObject *api);

	// Unregister API (similar to ScopyJS::unregisterApi)
	static void unregisterApi(ApiObject *api);

	// Type-safe API access with availability checking
	template <typename T>
	static T *getApi(const QString &objectName)
	{
		QMutexLocker locker(&s_mutex);
		auto it = s_apis.find(objectName);
		if(it == s_apis.end() || it->isNull()) {
			return nullptr;
		}
		return qobject_cast<T *>(it->data());
	}

	// Check if API is available
	static bool isAvailable(const QString &objectName);

	// Get list of available APIs
	static QStringList availableApis();

private:
	static QMap<QString, QPointer<ApiObject>> s_apis;
	static QMutex s_mutex;

	// Auto-cleanup when API object is destroyed
	static void onApiDestroyed();
};

} // namespace scopy

#endif // APILIST_H