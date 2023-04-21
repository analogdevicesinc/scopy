#include "mainwindow.hpp"
#include "src/logging_categories.h"

#include <QApplication>
#include <qfile.h>

int main(int argc, char *argv[])
{
	SetScopyQDebugMessagePattern();
	QLoggingCategory::setFilterRules(""
//					 "*.debug=true\n"
					 "default=false"
//					  "ScanContextCollector.debug=true"
					 );

	QApplication a(argc, argv);
	QString path("/home/ubuntu/Documents/default.qss");
	QFile file(path);
	file.open(QFile::ReadOnly);
	QString stylesheet = QString::fromLatin1(file.readAll());
	a.setStyleSheet(stylesheet);
	MainWindow w;
	w.show();
	return a.exec();
}
