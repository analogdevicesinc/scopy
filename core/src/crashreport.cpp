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

#include "crashreport.h"
#include <fstream>
#include <signal.h>
#include <stdlib.h>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <common/scopyconfig.h>
#include "logging_categories.h"

Q_LOGGING_CATEGORY(CAT_CRASH_REPORT, "CrashReport")

using namespace scopy;

QString CrashReport::tmpFilePath_;

void CrashReport::signalHandler(int)
{
	::signal(SIGSEGV, SIG_DFL);
	::signal(SIGABRT, SIG_DFL);
	qSetMessagePattern("[ "
#ifdef QCRITICAL_BACKTRACE_STR
			   QCRITICAL_BACKTRACE_STR " "
#endif
#ifdef QDEBUG_LOG_TIME
			   QDEBUG_LOG_TIME_STR
#endif
			   " ] "
			   " - "
			   "%{message}");

	qCritical(CAT_CRASH_REPORT) << "Scopy finished with error\n";
}

void CrashReport::initSignalHandler()
{
	tmpFilePath_ = scopy::config::tempLogFilePath();
	QString qd = scopy::config::settingsFolderPath();

	QFileInfo previousLog(tmpFilePath_);

	if(previousLog.exists()) {
		qInfo(CAT_CRASH_REPORT) << "Found existing crash stack trace";
		QString dumpDateAndTime =
			previousLog.lastModified().toString(Qt::ISODate).replace("T", "--").replace(":", "-");
		QString currentLog = QDir::cleanPath(qd + "/" + "ScopyCrashDump--" + dumpDateAndTime + ".log");
		bool renamed = QFile::rename(previousLog.filePath(), currentLog);
		if(renamed) {
			qInfo(CAT_CRASH_REPORT) << "Successfully renamed crash log to: " << currentLog;
		}
		QFile::remove(tmpFilePath_);
	}

	signal(SIGILL, &signalHandler);
	signal(SIGSEGV, &signalHandler);
	signal(SIGABRT, &signalHandler);
	signal(SIGFPE, &signalHandler);
}
