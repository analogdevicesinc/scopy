#include "core/scopymainwindow.h"
#include "core/logging_categories.h"
#include <QApplication>
#include <core/application_restarter.h>
#include <gui/utils.h>


void SetScopyQDebugMessagePattern() {

	qSetMessagePattern(
		"[ "
		#ifdef QDEBUG_LOG_MSG_TYPE
			QDEBUG_LOG_MSG_TYPE_STR " "
			QDEBUG_CATEGORY_STR " "
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
		"%{message}"
		);
}

int main(int argc, char *argv[])
{
	SetScopyQDebugMessagePattern();
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
					 "ScopyTranslations.debug=true\n"
					 );

	QCoreApplication::setOrganizationName("ADI");
	QCoreApplication::setOrganizationDomain("analog.com");
	QCoreApplication::setApplicationName("Scopy-v2");
	QSettings::setDefaultFormat(QSettings::IniFormat);

	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts,true);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);

	scopy::ApplicationRestarter restarter(QString::fromLocal8Bit(argv[0]));
	a.setWindowIcon(QIcon(":/gui/icon.ico"));
	a.setStyleSheet(Util::loadStylesheetFromFile(":/gui/stylesheets/default.qss"));
	scopy::ScopyMainWindow w;
	w.show();
	int ret = a.exec();
	restarter.restart(ret);
	printf("Scopy finished gracefully\n");
	return ret;
}
