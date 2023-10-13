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
	exit(EXIT_FAILURE);
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
