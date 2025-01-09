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

#ifndef SCOPYBENCHMARK_H
#define SCOPYBENCHMARK_H

#include <QElapsedTimer>
#include <QString>
#include "scopy-common_export.h"

#define CONSOLE_LOG(logger, msg) logger.log(msg, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define CONSOLE_LOG_RESET(logger, msg) logger.logAndReset(msg, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define FILE_LOG(logger, msg, path) logger.log(path, msg, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define FILE_LOG_RESET(logger, msg, path) logger.logAndReset(path, msg, __PRETTY_FUNCTION__, __FILE__, __LINE__)

namespace scopy {
class SCOPY_COMMON_EXPORT ScopyBenchmark
{
public:
	ScopyBenchmark();
	~ScopyBenchmark();

	void startTimer();
	void restartTimer();

	void log(const QString &msg, const char *function, const char *file, int line);
	void log(const QString &filePath, const QString &msg, const char *function, const char *file, int line);
	void logAndReset(const QString &msg, const char *function, const char *file, int line);
	void logAndReset(const QString &filePath, const QString &msg, const char *function, const char *file, int line);

private:
	QElapsedTimer m_timer;
};

} // namespace scopy

#endif // SCOPYBENCHMARK_H
