#include "scopymainwindow.h"
#include "logging_categories.h"

#include <QApplication>
#include <utils.h>

int main(int argc, char *argv[])
{
	SetScopyQDebugMessagePattern();
	QLoggingCategory::setFilterRules(""
//					 "*.debug=false\n"
//					  "ToolManager.debug=true"
					 );

	QApplication a(argc, argv);
	a.setStyleSheet(Util::loadStylesheetFromFile(":/stylesheets/default.qss"));
	adiscope::ScopyMainWindow w;
	w.show();
	int ret = a.exec();
	printf("Scopy finished gracefully\n");
	return ret;
}
