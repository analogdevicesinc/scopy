/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QSettings>
#include <QtGlobal>
#include <QProcess>
#include <QDir>
#include <QDateTime>

#include "config.h"
#include "tool_launcher.hpp"

#define __STDC_FORMAT_MACROS
#ifdef Q_OS_WIN
#include "breakpad/client/windows/handler/exception_handler.h"
#endif

#ifdef Q_OS_LINUX
#include "client/linux/handler/exception_handler.h"
#endif
using namespace adiscope;
using namespace google_breakpad;

static ExceptionHandler* handler = NULL;
#ifdef Q_OS_WIN
bool ShowDumpResults(const wchar_t* dump_path,
                     const wchar_t* minidump_id,
                     void* context,
                     EXCEPTION_POINTERS* exinfo,
                     MDRawAssertionInfo* assertion,
                     bool succeeded) {
    printf("Dump path: %s\n", dump_path);
    return succeeded;
}
#endif

#ifdef Q_OS_LINUX
static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
void* context, bool succeeded) {
    printf("Dump path: %s\n", descriptor.path());
    return succeeded;
}


#endif


QString initBreakPadHandler(QString crashDumpPath)
{
	QString prevCrashDump="";
	QString appDir = crashDumpPath;
	QDir qd(appDir);
	if(qd.exists()) {
		qd.mkdir("crashdmp");
	}
	qd.cd("crashdmp");
	QStringList dumps= qd.entryList();
	for(QString dump : dumps)
	{
		if(dump=="." || dump == "..") continue;
		if(dump.startsWith("ScopyCrashDump"))
			continue;
		QString dumpFullPath = qd.path()+qd.separator()+dump;
		QFileInfo fi(dumpFullPath);
		QString dumpDateAndTime = fi.lastModified().toString(Qt::ISODate).replace("T","--").replace(":","-");
		prevCrashDump = qd.path()+qd.separator()+"ScopyCrashDump--"+dumpDateAndTime+".dmp";
		QFile::rename(dumpFullPath, prevCrashDump);


	}

	QByteArray ba = qd.path().toLocal8Bit();
	const char *appDirStr = ba.data();

	static size_t google_custom_count = 3;
	static google_breakpad::CustomInfoEntry google_custom_entries[] = {
		google_breakpad::CustomInfoEntry(L"prod", L"Scopy"),
		google_breakpad::CustomInfoEntry(L"ver", L"1.0.3"),
	};

	google_breakpad::CustomClientInfo custom_info = {google_custom_entries,
													 google_custom_count};

#ifdef Q_OS_LINUX
	google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallback, NULL, true, -1);
#endif
#ifdef Q_OS_WIN
	handler = new google_breakpad::ExceptionHandler(qd.path().toStdWString()/*L"C:/dumps/"*/,
														 NULL,
														 NULL/*ShowDumpResults*/ ,
														 NULL,
														 google_breakpad::ExceptionHandler::HANDLER_ALL,
														 MiniDumpNormal,
														 (wchar_t*)NULL,
														 &custom_info/*NULL*/);
#endif
	return prevCrashDump;
}

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	QFont font("Open Sans");
	app.setFont(font);

	if (app.styleSheet().isEmpty()) {
		QFile file(":/stylesheets/stylesheets/global.qss");
		file.open(QFile::ReadOnly);

		QString stylesheet = QString::fromLatin1(file.readAll());
		app.setStyleSheet(stylesheet);
	}

	auto pythonpath = qgetenv("SCOPY_PYTHONPATH");
	if (!pythonpath.isNull())
		qputenv("PYTHONPATH", pythonpath);

	QCoreApplication::setOrganizationName("ADI");
	QCoreApplication::setOrganizationDomain("analog.com");
	QCoreApplication::setApplicationName("Scopy");
	QCoreApplication::setApplicationVersion(SCOPY_VERSION_GIT);
	QSettings::setDefaultFormat(QSettings::IniFormat);

	QSettings test;
	QString path = test.fileName();
	QString fn("Scopy.ini");
	path.chop(fn.length());

	QString prevCrashDump = initBreakPadHandler(path);

	QCommandLineParser parser;

	parser.addHelpOption();
	parser.addVersionOption();

	parser.addOptions({
		{ {"s", "script"}, "Run given script.", "script" },
		{ {"n", "nogui"}, "Run Scopy without GUI" },
		{ {"d", "nodecoders"}, "Run Scopy without digital decoders"}
	});

	parser.process(app);

	ToolLauncher launcher(prevCrashDump);

	bool nogui = parser.isSet("nogui");
	bool nodecoders = parser.isSet("nodecoders");
	if (nodecoders) {
		launcher.setUseDecoders(false);
	}
	QString script = parser.value("script");
	if (nogui) {
		launcher.hide();
	} else {
		launcher.show();
	}
	if (!script.isEmpty()) {
		QFile file(script);
		if (!file.open(QFile::ReadOnly)) {
			qCritical() << "Unable to open script file";
			return EXIT_FAILURE;
		}

		QTextStream stream(&file);

		QString firstLine = stream.readLine();
		if (!firstLine.startsWith("#!"))
			stream.seek(0);

		QString contents = stream.readAll();
		file.close();

		QMetaObject::invokeMethod(&launcher,
				 "runProgram",
				 Qt::QueuedConnection,
				 Q_ARG(QString, contents),
				 Q_ARG(QString, script));
	}

	return app.exec();
}
