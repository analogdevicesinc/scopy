#include <QFile>
#include <QIcon>

#include "theme_manager.hpp"
#include "utils.h"

using namespace scopy::gui;

ThemeManager::ThemeManager()
	: m_app(nullptr)
{
	m_availableThemes.append("default");
	m_availableThemes.append("light");
}

ThemeManager& ThemeManager::getInstance()
{
	static ThemeManager INSTANCE;
	return INSTANCE;
}

void ThemeManager::setApplication(QApplication* app) { m_app = app; }

void ThemeManager::setCurrentTheme(const QString& theme)
{
	m_currentTheme = theme;

	Util util;
	QString stylesheet = util.loadStylesheetFromFile(":/stylesheets/themes/" + theme + ".qss");

	if (m_app) {
		m_app->setStyleSheet(stylesheet);
	}

	QIcon::setThemeName(theme);
}

QString ThemeManager::getCurrentTheme() const { return m_currentTheme; }

const QStringList ThemeManager::getAvailableThemes() { return m_availableThemes; }
