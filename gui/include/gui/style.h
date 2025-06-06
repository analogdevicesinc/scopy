/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef STYLEREPOSITORY_H
#define STYLEREPOSITORY_H

#include <QColor>
#include <QFileInfoList>
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

	bool setTheme(QString theme, float fontScale);
	QString getThemeFromPkgs(QString theme);
	QString getStylePath(QString relativePath);
	void initPaths();
	void generateStyle();
	QString getAllProperties();
	QFileInfoList getQssList(QString path);
	static bool isProperty(QString style);
	static const char *replaceProperty(const char *prop);
	static QString scaleNumberInString(QString string, float factor);
	static QString adjustForScaling(QString key, QString value, float scale);
	static void adjustJsonForScaling(float scale);
	QString getThemePath(QString theme);

public:
	static Style *GetInstance();

	// initialize style. This must be done as soon as possible in the application's lifetime
	bool init(QString theme = "", float fontScale = 1);

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
	// force: will do setStyleSheet on the widget. This may be needed if another stylesheet was previously set on
	// this widget. using force is not recommended
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
	QStringList getThemeList();

	// recursively replaces keys from style_attributes.h
	// you shouldn't have to use this
	static QString replaceAttributes(QString style, int calls_limit = 10);

	// Load the themes and QSS from the packages directory
	static void setPkgsThemes(QFileInfoList infoList);
	static void setPkgsQss(QFileInfoList infoList);

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
	QString m_jsonFolderPath;

	static QFileInfoList m_pkgThemes;
	static QFileInfoList m_pkgQss;
};
} // namespace scopy

#endif // STYLEREPOSITORY_H
