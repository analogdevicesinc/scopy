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
					 "Plugin.debug=true"
					 );

	QCoreApplication::setOrganizationName("ADI");
	QCoreApplication::setOrganizationDomain("analog.com");
	QCoreApplication::setApplicationName("Scopy-v2");
	QSettings::setDefaultFormat(QSettings::IniFormat);

	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts,true);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);

	adiscope::ApplicationRestarter restarter(QString::fromLocal8Bit(argv[0]));
	a.setStyleSheet(Util::loadStylesheetFromFile(":/stylesheets/default.qss"));
	adiscope::ScopyMainWindow w;
	w.show();
	int ret = a.exec();
	restarter.restart(ret);
	printf("Scopy finished gracefully\n");
	return ret;
}
