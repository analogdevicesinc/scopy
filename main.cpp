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

#include "core/logging_categories.h"
#include "core/scopymainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <style.h>

#include <core/application_restarter.h>
#include <core/cmdlinehandler.h>
#include <core/scopymainwindow_api.h>
#include <gui/utils.h>
#include <gui/widgets/scopysplashscreen.h>
#include <core/crashreport.h>
#include <gui/utils.h>
#include <gui/docking/docksettings.h>

#ifdef __ANDROID__
	#include <QtAndroidExtras/QtAndroid>
#endif

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_RUNTIME_ENVIRONMENT_INFO, "RuntimeEnvironmentInfo")

void SetScopyQDebugMessagePattern()
{

	qSetMessagePattern("[ "
#ifdef QDEBUG_LOG_MSG_TYPE
			   QDEBUG_LOG_MSG_TYPE_STR " " QDEBUG_CATEGORY_STR " "
#endif
#ifdef QDEBUG_LOG_TIME
			   QDEBUG_LOG_TIME_STR
#endif
#ifdef QDEBUG_LOG_DATE
				   QDEBUG_LOG_DATE_STR
#endif
#ifdef QDEBUG_LOG_CATEGORY
					   QDEBUG_CATEGORY_STR
#endif
			   " ] "
#ifdef QDEBUG_LOG_FILE
			   QDEBUG_LOG_FILE_STR
#endif

			   " - "
			   "%{message}");
}

void initLogging()
{
	QLoggingCategory::setFilterRules(""
					 "*.debug=false\n"
					 "CyclicalTask.debug=false\n"
					 "ToolStack.debug=true\n"
					 "ToolManager.debug=true\n"
					 "DeviceManager.debug=true\n"
					 "Device.debug=true\n"
					 "Plugin.debug=true\n"
					 "PluginManager.debug=true\n"
					 "PluginRepository.debug=true\n"
					 "Scopy_API.debug=true\n"
					 "RuntimeEnvironmentInfo.debug=true\n"
					 "Style.debug=true\n"
					 "EmuAddPage.debug=true\n"
					 "Scopy.debug=true\n"
					 "ScopyPreferencesPage.debug=true\n");




	if(!getenv("QT_MESSAGE_PATTERN")) {
		SetScopyQDebugMessagePattern();
	}
}

void printRuntimeEnvironmentInfo()
{
	QStringList infoList = scopy::config::dump().split("\n");
	for(const QString &info : infoList) {
		qInfo(CAT_RUNTIME_ENVIRONMENT_INFO) << info;
	}
}

void printEnv()
{
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	for (const QString &key: env.keys()) {
		qDebug() << "-=-" << key <<"  "<< env.value(key);
	}
}

#ifdef __ANDROID__
void reqPermissions()
{
	const QVector<QString> Permissions({"android.permission.CLEAR_APP_USER_DATA",
					    "android.permission.CLEAR_APP_CACHE",
					    "android.permission.READ_EXTERNAL_STORAGE",
					    "android.permission.WRITE_EXTERNAL_STORAGE",
					    "android.permission.DELETE_CACHE_FILES",
					    "android.permission.ACCESS_NETWORK_STATE",
					    "android.permission.INTERNET",
					    "android.permission.WAKE_LOCK",
					    "android.permission.REQUEST_IGNORE_BATTERY_OPTIMIZATIONS"});

	for (const QString &Permission : Permissions) {
		switch(QtAndroid::checkPermission(Permission)) {
		case QtAndroid::PermissionResult::Denied: {
			qDebug() << Permission + " requested";
			QtAndroid::requestPermissionsSync(QStringList({Permission}));
			switch(QtAndroid::checkPermission(Permission)) {
			case QtAndroid::PermissionResult::Denied: qDebug() << Permission + " denied"; break;
			case QtAndroid::PermissionResult::Granted: qDebug() << Permission + " already granted"; break;
			}
			break;
		}
		case QtAndroid::PermissionResult::Granted: qDebug() << Permission + " already granted"; break;
		}
	}

}
#endif

int main(int argc, char *argv[])
{
#ifdef __APPLE__
	// this forces a newer version of OpenGL to work with Qt3DExtras
	QSurfaceFormat fmt;
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setVersion(3, 2);
	QSurfaceFormat::setDefaultFormat(fmt);
#endif

	QCoreApplication::setOrganizationName("ADI");
	QCoreApplication::setOrganizationDomain("analog.com");
	QCoreApplication::setApplicationName("Scopy-v2");
	QSettings::setDefaultFormat(QSettings::IniFormat);

	printRuntimeEnvironmentInfo();
	printEnv();
	initLogging();
	CrashReport::initSignalHandler();

	#ifdef  __ANDROID__

	QAndroidJniObject jniObject = QtAndroid::androidActivity().callObjectMethod("getScaleFactor", "()Ljava/lang/String;");
	QString scaleFactor = jniObject.toString();

	qputenv("QT_SCALE_FACTOR", scaleFactor.toUtf8());
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	qputenv("QT_USE_ANDROID_NATIVE_DIALOGS", "1");

	QApplication::setAttribute(Qt::AA_CompressHighFrequencyEvents, true);
	QApplication::setAttribute(Qt::AA_CompressTabletEvents, true);
	qputenv("SCOPY_USE_OPEN_GL", "1");
	#endif

	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
	QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);
	ScopySplashscreen::init();
	ScopySplashscreen::showMessage("Initializing scopy");

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOptions({
		{{"s", "script"}, "Run given script.", "script"},
		{{"S", "script-list"}, "Run given script list.", "script-list"},
		{{"r", "keep-running"}, "Keep the application session after running a certain script."},
		{{"a", "accept-license"}, "Accept the license in advance."},
		{{"l", "logfile"}, "Saves all the logging messages into a file.", "filename"},
		{{"c", "connect"}, "Establish the connection to a given device by URI.", "URI"},
		{{"t", "tool"}, "Select the desired tool for the device.", "tool"},
	});

	parser.process(a);
	// CmdLineHandler::withLogFileOption(parser);
	ApplicationRestarter restarter(QString::fromLocal8Bit(argv[0]));
	a.setWindowIcon(QIcon(":/gui/icon.ico"));
	scopy::initDockWidgets();
	ScopyMainWindow w;
	w.show();


	ScopyMainWindow_API scopyApi(&w);
	int retHandler = CmdLineHandler::handle(parser, scopyApi);
	if(retHandler == EXIT_FAILURE) {
		return retHandler;
	}
	ScopySplashscreen::finish(&w);
	int ret = a.exec();
	restarter.restart(ret);
	printf("Scopy finished gracefully\n");
	CmdLineHandler::closeLogFile();
	return ret;
}
