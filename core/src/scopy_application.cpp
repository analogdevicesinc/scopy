/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <scopy/core/scopy_application.hpp>

bool ScopyApplication::getDebugMode() const { return debugMode; }

void ScopyApplication::setDebugMode(bool value) { debugMode = value; }

ScopyApplication::ScopyApplication(int& argc, char** argv)
	: QApplication(argc, argv)
	, debugMode(false)
{}

#if BREAKPAD_HANDLER
using namespace google_breakpad;
QString ScopyApplication::initBreakPadHandler(QString crashDumpPath)
{
	QString prevCrashDump = "";
	QString appDir = crashDumpPath;
	QDir qd(appDir);
	if (qd.exists()) {
		qd.mkdir("crashdmp");
	}
	qd.cd("crashdmp");
	QStringList dumps = qd.entryList();
	for (QString dump : dumps) {
		if (dump == "." || dump == "..")
			continue;
		if (dump.startsWith("ScopyCrashDump"))
			continue;
		QString dumpFullPath = qd.path() + qd.separator() + dump;
		QFileInfo fi(dumpFullPath);
		QString dumpDateAndTime = fi.lastModified().toString(Qt::ISODate).replace("T", "--").replace(":", "-");
		prevCrashDump = qd.path() + qd.separator() + "ScopyCrashDump--" + dumpDateAndTime + ".dmp";
		QFile::rename(dumpFullPath, prevCrashDump);
	}

#ifdef Q_OS_LINUX
	descriptor = new google_breakpad::MinidumpDescriptor(qd.path().toStdString().c_str());
	handler->set_minidump_descriptor(*descriptor);
#endif
#ifdef Q_OS_WIN
	handler->set_dump_path(qd.path().toStdWString());
#endif
	return prevCrashDump;
}

ScopyApplication::~ScopyApplication()
{
#ifdef Q_OS_LINUX
	delete descriptor;
#endif
}

#ifdef CATCH_UNHANDLED_EXCEPTIONS
bool ScopyApplication::notify(QObject* receiver, QEvent* e)
{

	try {
		return QApplication::notify(receiver, e);
		// Handle the desired exception type
	} catch (...) {
		// Handle the rest
		handler->WriteMinidump();
		std::terminate();
	}
	return false;
}
#endif

#ifdef Q_OS_WIN
bool ScopyApplication::dumpCallback(const wchar_t* dump_path, const wchar_t* minidump_id, void* context,
				    EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
{
	printf("Dump path: %s\n", dump_path);
	return succeeded;
}
#endif
ExceptionHandler* ScopyApplication::getExceptionHandler() const { return handler; }

void ScopyApplication::setExceptionHandler(ExceptionHandler* value) { handler = value; }

#ifdef Q_OS_LINUX
bool ScopyApplication::dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context,
				    bool succeeded)
{
	printf("Dump path: %s\n", descriptor.path());
	return succeeded;
}
#endif
#endif
