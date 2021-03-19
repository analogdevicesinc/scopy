#include <QFile>
#include <QIcon>

#include <scopy/gui/theme_manager.hpp>

using namespace scopy::gui;

ThemeManager::ThemeManager():
	m_app(nullptr)
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

	// TODO: replace this with already defined logic from utils
	QFile file(":/stylesheets/themes/" + theme + ".qss");
	file.open(QFile::ReadOnly);
	QString stylesheet = QString::fromLatin1(file.readAll());

	if(m_app) {
		m_app->setStyleSheet(stylesheet);
	}

	QIcon::setThemeName(theme);
}

QString ThemeManager::getCurrentTheme() const { return m_currentTheme; }

const QStringList ThemeManager::getAvailableThemes() { return m_availableThemes; }
