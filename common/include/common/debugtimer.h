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

#ifndef DEBUGTIMER_H
#define DEBUGTIMER_H

#include <QElapsedTimer>
#include <QString>
#include <QFile>
#include "scopy-common_export.h"

#define DEBUGTIMER_LOG(logger, msg) logger.log(msg, __PRETTY_FUNCTION__, __FILE__, __LINE__)

namespace scopy {
class SCOPY_COMMON_EXPORT DebugTimer
{
public:
	DebugTimer(QString filePath = "");
	~DebugTimer();

	void startTimer();
	void restartTimer();

	void setSingleMode(bool b);
	bool singleMode();

	void log(const QString &msg, const char *function, const char *file, int line);

private:
	QElapsedTimer m_timer;
	QString m_filePath;
	bool m_singleMode;
	QFile f;
};

} // namespace scopy

#endif // DEBUGTIMER_H
