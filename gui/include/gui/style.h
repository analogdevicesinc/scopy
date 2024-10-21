#ifndef STYLEREPOSITORY_H
#define STYLEREPOSITORY_H

#include <QColor>
#include <QObject>
#include "qvariant.h"
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
	QString getStylePath(QString relativePath);
	void initPaths();
	void genrateStyle();
	QString getAllProperties();
	static bool isProperty(QString style);
	static const char *replaceProperty(const char *prop);

public:
	static Style *GetInstance();
	static QString getAttribute(const char *key);
	static QColor getColor(const char *key);
	static int getDimension(const char *key);
	static void setStyle(QWidget *widget, const char *style, QVariant value = true, bool force = false);
	static QPixmap getPixmap(QString pixmap, QColor color = QColor());
	static QString getColorTransparent(const char *key, double transparency);
	static QColor getChannelColor(int index);
	static QList<QColor> getChannelColorList();
	static void setBackgroundColor(QWidget *widget, const char *color, bool extend_to_children = false);
	static void setBackgroundColor(QWidget *widget, QString color, bool extend_to_children = false);

	QString getTheme();
	bool setTheme(QString theme);
	QStringList getThemeList();

	static QString replaceAttributes(QString style, int calls_limit = 10);

	void setM2KStylesheet(QWidget *widget);
	void setGlobalStyle(QWidget *widget = nullptr);

private:
	static Style *pinstance_;
	static QJsonDocument *m_global_json;
	static QJsonDocument *m_theme_json;
	static QMap<QString, QString> *m_styleMap;
	QString m_globalJsonPath;
	QString m_themeJsonPath;
	QString m_qssGlobalFile;
	QString m_qssFolderPath;
	QString m_m2kqssFile;
};
} // namespace scopy

#endif // STYLEREPOSITORY_H
