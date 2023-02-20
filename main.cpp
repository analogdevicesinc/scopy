#include "scopymainwindow.h"
#include "logging_categories.h"

#include <QApplication>
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
//					 "*.debug=false\n"
					 "DeviceManager.debug=true\n"
					 "Device.debug=true\n"
					 "TestPlugin.debug=true\n"
					 "Plugin.debug=true"
					 );

	QApplication a(argc, argv);
	a.setStyleSheet(Util::loadStylesheetFromFile(":/stylesheets/default.qss"));
	adiscope::ScopyMainWindow w;
	w.show();
	int ret = a.exec();
	printf("Scopy finished gracefully\n");
	return ret;
}
