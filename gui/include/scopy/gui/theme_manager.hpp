#ifndef THEMEMANAGER_HPP
#define THEMEMANAGER_HPP

#include <QApplication>

namespace scopy {
namespace gui {

class ThemeManager
{
public:
	static ThemeManager& getInstance();

	ThemeManager(const ThemeManager&) = delete;
	ThemeManager& operator=(const ThemeManager&) = delete;

	void setApplication(QApplication* app);
	const QStringList getAvailableThemes();

	void setCurrentTheme(const QString& theme);
	QString getCurrentTheme() const;


private:
	ThemeManager();

	QApplication* m_app;
	QString m_currentTheme;
	QStringList m_availableThemes;
};
} // namespace gui
} // namespace scopy

#endif // THEMEMANAGER_HPP
