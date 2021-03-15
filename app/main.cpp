#include "mainwindow.h"

#include <QApplication>

#include <scopy/gui/theme_manager.hpp>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	QString theme = "default";

	scopy::gui::ThemeManager::getInstance().setApplication(&a);
	scopy::gui::ThemeManager::getInstance().setCurrentTheme(theme);


	MainWindow w;
	w.show();

	return a.exec();
}
