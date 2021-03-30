#include "mainwindow.h"

#include <QApplication>

#include <scopy/gui/theme_manager.hpp>
#include <scopy/gui/tool_launcher.hpp>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	QString theme = "default-theme";

	scopy::gui::ThemeManager::getInstance().setApplication(&a);
	scopy::gui::ThemeManager::getInstance().setCurrentTheme(theme);

	MainWindow w;
	w.show();

//	scopy::gui::ToolLauncher tl;
//	tl.show();

	return a.exec();
}
