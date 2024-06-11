#ifndef STYLEREPOSITORY_H
#define STYLEREPOSITORY_H

#include <QColor>
#include <QObject>
#include "scopy-gui_export.h"
#include <../gui/style_properties.h>
#include <../gui/style_attributes.h>

namespace scopy {
class SCOPY_GUI_EXPORT Style : public QObject
{
	Q_OBJECT
private:
	Style(QObject *parent = nullptr);
	~Style();

	void init(QString theme = "");
	void applyStyle();

public:
	static Style *GetInstance();
	static QString getAttribute(const char *key);
	static QColor getColor(const char *key);
	static int getDimension(const char *key);
	static void setStyle(QWidget *widget, const char *style);
	static QPixmap getPixmap(QString pixmap, QColor color = QColor());
	QString getTheme();
	bool setTheme(QString theme);
	QStringList getThemeList();
	static QString replaceAttributes(QString style, int calls_limit = 10);
	void setM2KStylesheet(QWidget *widget);

private:
	static Style *pinstance_;
	static QJsonDocument *m_global_json;
	static QJsonDocument *m_theme_json;
	QString m_globalJsonPath;
	QString m_themeJsonPath;
	QString m_qssGlobalFilePath;
	QString m_qssFolderPath;
	QString m_m2kqssPath;
	QString m_currentStyle;
};
} // namespace scopy

#endif // STYLEREPOSITORY_H
