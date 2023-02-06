#include "scopymainwindow.h"
#include "logging_categories.h"

#include <QApplication>
#include <utils.h>

int main(int argc, char *argv[])
{
	SetScopyQDebugMessagePattern();

	QApplication a(argc, argv);
	a.setStyleSheet(Util::loadStylesheetFromFile(":/stylesheets/default.qss"));
	adiscope::ScopyMainWindow w;
	w.show();
	return a.exec();
}
