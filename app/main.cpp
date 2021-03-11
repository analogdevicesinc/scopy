#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QFile file(":/stylesheets/global.qss");
	file.open(QFile::ReadOnly);
	QString stylesheet = QString::fromLatin1(file.readAll());
	a.setStyleSheet(stylesheet);

	MainWindow w;
	w.show();
	return a.exec();
}
