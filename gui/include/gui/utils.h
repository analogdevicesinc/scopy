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

#include <qevent.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <QWidget>
#include <QDockWidget>
#include <QMainWindow>

#include <string>
#include <algorithm>
#include <QDebug>
#include <QSizePolicy>
#include "scopy-gui_export.h"

class SCOPY_GUI_EXPORT Util
{
public:
	static void retainWidgetSizeWhenHidden(QWidget *w, bool retain = true);
	static void setWidgetNrOfChars(QWidget *w, int minNrOfChars, int maxNrOfChars=0);
	static QString loadStylesheetFromFile(const QString &path);
	static bool compareNatural(const std::string &a, const std::string &b);
	static QWidget* findContainingWindow(QWidget* w);
    static const QList<QString> getColors() {return {"#F44336", "#4DAF50", "#E91E63", "#9C27B0",
                                                     "#3F51B5", "#2096F3", "#795448", "#02BCD4",
                                                     "#019688", "#673AB7", "#8BC34A", "#CDDC39",
                                                     "#FFEB3B", "#FFC108", "#FF9800", "#FF5721",
                                                     "#03A9F4", "#9E9E9E", "#607D8B", "#FFFFFF"
                                                    };
                                            }
};

#define PLOT_MENU_BAR_ENABLED

class SCOPY_GUI_EXPORT DockerUtils : public QObject
{
public:
	static QDockWidget* createDockWidget(QMainWindow* mainWindow, QWidget* widget, const QString& title = "");
	static void configureTopBar(QDockWidget* docker);
};

#endif /* M2K_UTILS_H */
