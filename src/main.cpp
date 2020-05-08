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

#include <QApplication>
#include <QCommandLineParser>
#include <QSettings>
#include <QtGlobal>
#include <QProcess>
#include <QDir>
#include <QDateTime>
#include <QFontDatabase>
#include <QTranslator>
#include <QLocale>
#include "config.h"
#include "tool_launcher.hpp"
#include "scopyApplication.hpp"


using namespace adiscope;

int main(int argc, char **argv)
{
#if BREAKPAD_HANDLER
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
#else
	QApplication app(argc, argv);
#endif

	QFontDatabase::addApplicationFont(":/open-sans-regular.ttf");
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

#if BREAKPAD_HANDLER
	QString prevCrashDump = app.initBreakPadHandler(path);
#else
	QString prevCrashDump = "";
#endif


	QCommandLineParser parser;

	parser.addHelpOption();
	parser.addVersionOption();

	parser.addOptions({
		{ {"s", "script"}, "Run given script.", "script" },
		{ {"n", "nogui"}, "Run Scopy without GUI" },
		{ {"d", "nodecoders"}, "Run Scopy without digital decoders"},
		{ {"nd", "nonativedialog"}, "Run Scopy without native file dialogs"}
	});

	parser.process(app);

	QTranslator myappTranslator;

	QSettings pref(Preferences::getPreferenceIniFile(), QSettings::IniFormat);
	QString language = pref.value(QString("Preferences/language")).toString();

	QString languageFileName;
	QString osLanguage = QLocale::system().name().split("_")[0];
	QDir directory(QCoreApplication::applicationDirPath()+"/resources/languages");

	QStringList languages = directory.entryList(QStringList() << "*.qm",QDir::Files);
	if(language == "auto"){
		if(languages.contains("scopy_"+osLanguage+".qm")) {
			languageFileName = QDir(QCoreApplication::applicationDirPath()+"/resources/languages/scopy_"+osLanguage+".qm").path();
		}
		else{
			languageFileName = QDir(QCoreApplication::applicationDirPath()+"/resources/languages/scopy_en.qm").path();
		}
	}
	else if(languages.contains(language+".qm")) {
		languageFileName=QDir(QCoreApplication::applicationDirPath()+"/resources/languages/"+language+".qm").path();
	}
	else { 
		languageFileName = language;
	}

	myappTranslator.load(languageFileName);
	app.installTranslator(&myappTranslator);


	ToolLauncher launcher(prevCrashDump);

	bool nogui = parser.isSet("nogui");
	bool nodecoders = parser.isSet("nodecoders");
	if (nodecoders) {
		launcher.setUseDecoders(false);
	}

	bool nonativedialog = parser.isSet("nonativedialog");
#ifdef NONATIVE
    nonativedialog = true;
#endif
	qDebug() << "Using" << (nonativedialog ? "Qt" : "Native") << "file dialogs";
	launcher.setNativeDialogs(!nonativedialog);

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

