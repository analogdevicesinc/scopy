/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include "scopy-gui_export.h"

#include <QDebug>
#include <QDockWidget>
#include <QMainWindow>
#include <QPainter>
#include <QSizePolicy>
#include <QStyle>
#include <QStyleOption>
#include <QWidget>
#include <QtXml/QDomElement>
#include <qevent.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_picker.h>

#include <algorithm>
#include <string>

class SCOPY_GUI_EXPORT Util
{
public:
	typedef struct
	{
		QString tag;
		QString id;
		QString attrName;
		QString attrVal;
	} SVGSpec;

	static void retainWidgetSizeWhenHidden(QWidget *w, bool retain = true);
	static void setWidgetNrOfChars(QWidget *w, int minNrOfChars, int maxNrOfChars = 0);
	static QString loadStylesheetFromFile(const QString &path);
	static bool compareNatural(const std::string &a, const std::string &b);
	static QString doubleToQString(double value);
	static QWidget *findContainingWindow(QWidget *w);
	static void SetAttrRecur(QDomElement &elem, SVGSpec s);
	static QPixmap ChangeSVGColor(QString iconPath, QString color, float opacity);
	static QPixmap ChangeSVGAttr(QString iconPath, QList<SVGSpec> list);
};

#define PLOT_MENU_BAR_ENABLED

class SCOPY_GUI_EXPORT DockerUtils : public QObject
{
public:
	static QDockWidget *createDockWidget(QMainWindow *mainWindow, QWidget *widget, const QString &title = "");
	static void configureTopBar(QDockWidget *docker);
};

// https://forum.qt.io/topic/25142/solved-applying-style-on-derived-widget-with-custom-property-failes/2
// https://doc.qt.io/qt-5/stylesheet-reference.html

#define QWIDGET_PAINT_EVENT_HELPER                                                                                     \
public:                                                                                                                \
	void paintEvent(QPaintEvent *e) override                                                                       \
	{                                                                                                              \
		QStyleOption opt;                                                                                      \
		opt.init(this);                                                                                        \
		QPainter p(this);                                                                                      \
		style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);                                             \
	}

#endif /* GUI_UTILS_H */
