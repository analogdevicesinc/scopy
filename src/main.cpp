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
#include "scopyApplication.hpp"

using namespace adiscope;

int main(int argc, char **argv)
{
#ifdef Q_OS_LINUX
	google_breakpad::MinidumpDescriptor descriptor("/tmp");
	google_breakpad::ExceptionHandler eh(descriptor, NULL, ScopyApplication::dumpCallback, NULL, true, -1);
#endif

#ifdef Q_OS_WIN
	google_breakpad::ExceptionHandler eh(L"C:/dumps/",
										NULL,
										ScopyApplication::dumpCallback,
										NULL,
										google_breakpad::ExceptionHandler::HANDLER_ALL,
										MiniDumpNormal,
										(wchar_t*)NULL,
										NULL);
#endif

	ScopyApplication app(argc, argv);
	app.setExceptionHandler(&eh);

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

	QString prevCrashDump = app.initBreakPadHandler(path);

	QCommandLineParser parser;

	parser.addHelpOption();
	parser.addVersionOption();

	parser.addOptions({
		{ {"s", "script"}, "Run given script.", "script" },
		{ {"n", "nogui"}, "Run Scopy without GUI" },
	});

	parser.process(app);

	ToolLauncher launcher(prevCrashDump);

	bool nogui = parser.isSet("nogui");
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
