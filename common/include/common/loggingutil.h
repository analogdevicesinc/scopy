/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef LOGGINGUTIL_H
#define LOGGINGUTIL_H

#include <QLoggingCategory>
#include <QString>
#include <pluginbase/statusbarmanager.h>
#include "scopy-common_export.h"

namespace scopy {

class SCOPY_COMMON_EXPORT LoggingUtil
{
public:
	enum LogLevel
	{
		Info,
		Warning,
		Debug
	};

	static inline void logMessage(const QLoggingCategory &category(), const QString &message,
				      LogLevel level = LogLevel::Debug, bool pushToStatusBar = false,
				      bool urgent = false, int ms = DEFAULT_DISPLAY_TIME)
	{
		if(pushToStatusBar) {
			if(urgent) {
				StatusBarManager::pushUrgentMessage(message, ms);
			} else {
				StatusBarManager::pushMessage(message, ms);
			}
		}

		switch(level) {
		case Info:
			qInfo(category) << message;
			break;
		case Warning:
			qWarning(category) << message;
			break;
		case Debug:
			qDebug(category) << message;
			break;
		}
	}
};

} // namespace scopy

#endif // LOGGINGUTIL_H
