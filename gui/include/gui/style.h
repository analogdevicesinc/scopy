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

	// get value from json file using the keys in style_attributes.h as QString
	static QString getAttribute(const char *key);

	// get color from json file using the keys in style_attributes.h as QColor
	// value needs to be a color code
	static QColor getColor(const char *key);

	// get dimension from json file using the keys in style_attributes.h as int
	// value needs to be an intiger
	static int getDimension(const char *key);

	// set style to a widget using keys in style_properties.h
	// value: the property value the widget will have. Only change it if the style css code uses different values
	// force: will do setStyleSheet on the widget. This may be needed if another stylesheet was previously set on this widget.
	// using force is not recommended
	static void setStyle(QWidget *widget, const char *style, QVariant value = true, bool force = false);

	// get pixmap with color changed to json::theme::pixmap_color
	// color: used for setting specific colors
	static QPixmap getPixmap(QString pixmap, QColor color = QColor());

	// get transparent color based on color picked from style_attributes.h
	// value needs to be a color code
	// transparency: percentage value 0-100
	static QString getColorTransparent(const char *key, double transparency);

	// get ch color for index
	// THIS IS LIMITED TO CHANNELS IN JSON, if the index is too large use StyleHelper
	static QColor getChannelColor(int index);

	// get all ch colors
	static QList<QColor> getChannelColorList();

	// set background to widget
	// color: keys in style_attributes.h or QString color code
	// extend_to_children: using false is recommended
	static void setBackgroundColor(QWidget *widget, const char *color, bool extend_to_children = false);
	static void setBackgroundColor(QWidget *widget, QString color, bool extend_to_children = false);

	QString getTheme();
	bool setTheme(QString theme);
	QStringList getThemeList();

	// recursively replaces keys from style_attributes.h
	// you shouldn't have to use this
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
