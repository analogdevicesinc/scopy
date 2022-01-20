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
#include <stdio.h>
#ifdef LIBM2K_ENABLE_LOG
#include <glog/logging.h>
#endif

//#include "coloreditor.h"
#include "scopy_color_editor.h"
#include "application_restarter.h"

#ifdef __ANDROID__
	#include <QtAndroidExtras/QtAndroid>
#endif

using namespace adiscope;

int main(int argc, char **argv)
{
#ifdef  __ANDROID__
	QAndroidJniObject jniObject = QtAndroid::androidActivity().callObjectMethod("getScaleFactor", "()Ljava/lang/String;");
	QString scaleFactor = jniObject.toString();

	qputenv("QT_SCALE_FACTOR", scaleFactor.toUtf8());
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QApplication::setAttribute(Qt::AA_CompressHighFrequencyEvents, true);
	QApplication::setAttribute(Qt::AA_CompressTabletEvents, true);
#endif

	ScopyApplication app(argc, argv);
#ifdef LIBM2K_ENABLE_LOG
	enableLogging(true);
	google::InitGoogleLogging(argv[0]);
	FLAGS_logbuflevel = -1;
#endif

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
	app.setExceptionHandler(&eh);
#endif

	QFontDatabase::addApplicationFont(":/open-sans-regular.ttf");
	QFont font("Open Sans");
	app.setFont(font);

	ApplicationRestarter restarter(QString::fromLocal8Bit(argv[0]));

#ifdef WIN32
	auto pythonpath = qgetenv("SCOPY_PYTHONPATH");
	auto path_str = QCoreApplication::applicationDirPath() + "\\" + PYTHON_VERSION + ";";
	path_str += QCoreApplication::applicationDirPath() + "\\" + PYTHON_VERSION + "\\plat-win;";
	path_str += QCoreApplication::applicationDirPath() + "\\" + PYTHON_VERSION + "\\lib-dynload;";
	path_str += QCoreApplication::applicationDirPath() + "\\" + PYTHON_VERSION + "\\site-packages;";
	path_str += QString::fromLocal8Bit(pythonpath);
	qputenv("PYTHONPATH", path_str.toLocal8Bit());
#endif

	QCoreApplication::setOrganizationName("ADI");
	QCoreApplication::setOrganizationDomain("analog.com");
	QCoreApplication::setApplicationName("Scopy");
	QCoreApplication::setApplicationVersion(SCOPY_VERSION_GIT);
	QSettings::setDefaultFormat(QSettings::IniFormat);

#if BREAKPAD_HANDLER
	QSettings test;
	QString path = test.fileName();
	QString fn("Scopy.ini");
	path.chop(fn.length());
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
					{ {"nd", "nonativedialog"}, "Run Scopy without native file dialogs"},
					{ {"og", "opengl"}, "Force use OpenGL for plots"} ,
					{ {"nog", "noopengl"}, "Force software rendering for plots"} ,
			  });

	parser.process(app);

	restarter.setArguments(parser.positionalArguments());

	QTranslator myappTranslator;

	// TODO: Use Preferences_API to get language key - cannot be done right now
	// as this involves instantiating Preferences object
	QSettings pref(Preferences::getPreferenceIniFile(), QSettings::IniFormat);
	QString language = pref.value(QString("Preferences/language")).toString();

	QString languageFileName = ":/translations/";
	QString osLanguage = QLocale::system().name().split("_")[0];

	QDir directory(":/translations");
	QStringList languages = directory.entryList(QStringList() << "*.qm" << "*.QM", QDir::Files);

	if(languages.contains(language+".qm")) {
		// use one of the precompiled languages - no .qm extension
		languageFileName += language+".qm";
	} else if(language == "auto") {
		// use auto from precompiled languages using the system locale
		languageFileName += osLanguage + ".qm";
	} else {
		// use language using absolute path
		languageFileName = language;
	}

	myappTranslator.load(languageFileName);
	app.installTranslator(&myappTranslator);

	ScopyColorEditor *colorEditor = new ScopyColorEditor(&app);
	colorEditor->setVisible(false);

	QString currentStylesheet = pref.value(QString("Preferences/currentStylesheet")).toString();
	QStringList userStylesheets = pref.value(QString("Preferences/userStylesheets")).toStringList();
	colorEditor->setUserStylesheets(userStylesheets);
	colorEditor->setCurrentStylesheet(currentStylesheet);

	if (app.styleSheet().isEmpty()) {
		app.setStyleSheet(colorEditor->getStyleSheet());
	}

	bool openGl = parser.isSet("opengl");
	bool noOpenGl = parser.isSet("noopengl");
	if(openGl && noOpenGl) {
		qDebug()<<"Ambigous openGL parameters";
		return -1;
	}
	if(openGl) {
		qputenv("SCOPY_USE_OPENGL","1");
	}
	if(noOpenGl) {
		qputenv("SCOPY_USE_OPENGL","0");
	}

	ToolLauncher launcher(prevCrashDump);
	launcher.getPrefPanel()->setColorEditor(colorEditor);

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
	return restarter.restart(app.exec());
}

