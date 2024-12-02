#include "core/logging_categories.h"
#include "core/scopymainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

#include <core/application_restarter.h>
#include <core/cmdlinehandler.h>
#include <core/scopymainwindow_api.h>
#include <gui/utils.h>
#include <core/crashreport.h>

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
	if(!getenv("QT_LOGGING_RULES")) {
		QLoggingCategory::setFilterRules(""
						 "*.debug=false\n"
						 "ToolStack.debug=true\n"
						 "ToolManager.debug=true\n"
						 "DeviceManager.debug=true\n"
						 "Device.debug=true\n"
						 "TestPlugin.debug=true\n"
						 "Plugin.debug=true\n"
						 "swiotConfig.debug=true\n"
						 "CyclicalTask.debug=false\n"
						 "SWIOTPlugin.debug=true\n"
						 "AD74413R.debug=true\n"
						 "ScopyTranslations.debug=true\n");
	}
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

int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName("ADI");
	QCoreApplication::setOrganizationDomain("analog.com");
	QCoreApplication::setApplicationName("Scopy-v2");
	QSettings::setDefaultFormat(QSettings::IniFormat);

	initLogging();
	CrashReport::initSignalHandler();

	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);
	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOptions({
		{{"s", "script"}, "Run given script.", "script"},
		{{"r", "keep-running"}, "Keep the application session after running a certain script."},
		{{"a", "accept-license"}, "Accept the license in advance."},
		{{"l", "logfile"}, "Saves all the logging messages into a file.", "filename"},
		{{"c", "connect"}, "Establish the connection to a given device by URI.", "URI"},
		{{"t", "tool"}, "Select the desired tool for the device.", "tool"},
	});

	parser.process(a);
	CmdLineHandler::withLogFileOption(parser);

	printRuntimeEnvironmentInfo();
	ApplicationRestarter restarter(QString::fromLocal8Bit(argv[0]));
	a.setWindowIcon(QIcon(":/gui/icon.ico"));
	a.setStyle("Fusion");
	a.setStyleSheet(Util::loadStylesheetFromFile(":/gui/stylesheets/default.qss"));
	ScopyMainWindow w;
	w.show();

	ScopyMainWindow_API scopyApi(&w);
	int retHandler = CmdLineHandler::handle(parser, scopyApi);
	if(retHandler == EXIT_FAILURE) {
		return retHandler;
	}

	int ret = a.exec();
	restarter.restart(ret);
	printf("Scopy finished gracefully\n");
	CmdLineHandler::closeLogFile();
	return ret;
}
