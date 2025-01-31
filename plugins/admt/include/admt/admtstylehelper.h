/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef ADMTSTYLEHELPER_H
#define ADMTSTYLEHELPER_H

#include "scopy-admt_export.h"
#include "stylehelper.h"
#include "style.h"

#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QIcon>
#include <QMap>
#include <QString>

#include <plotwidget.h>

namespace scopy {
namespace admt{
class SCOPY_ADMT_EXPORT ADMTStyleHelper : public QObject
{
    Q_OBJECT
protected:
    ADMTStyleHelper(QObject *parent = nullptr);
    ~ADMTStyleHelper();
public:
    // singleton
    ADMTStyleHelper(ADMTStyleHelper &other) = delete;
    void operator=(const ADMTStyleHelper &) = delete;
    static ADMTStyleHelper *GetInstance();
public:
    static void initColorMap();
    static QString getColor(QString id);
    static void TopContainerButtonStyle(QPushButton *btn, QString objectName = "");
    static void PlotWidgetStyle(PlotWidget *widget, QString objectName = "");
    static void ComboBoxStyle(QComboBox *widget, QString objectName = "");
    static void LineEditStyle(QLineEdit *widget, QString objectName = "");
    static void ColoredSquareCheckbox(QCheckBox *chk, QColor color, QString objectName = "");
    static void StartButtonStyle(QPushButton *btn, QString objectName = "");
    static void TabWidgetStyle(QTabWidget *widget, const QString& styleHelperColor = "ScopyBlue", QString objectName = "");
    static void TextStyle(QWidget *widget, const char *styleHelperColor = json::global::white, bool isBold = false, QString objectName = "");// void TextStyle(QWidget *widget, const QString& styleHelperColor, bool isBold = false, QString objectName = "");
    static void MenuSmallLabel(QLabel *label, QString objectName = "");
    static void LineStyle(QFrame *line, QString objectName = "");
    static void UIBackgroundStyle(QWidget *widget, QString objectName = "");
    static void GraphChannelStyle(QWidget *widget, QLayout *layout, QString objectName = "");
    static void CalculatedCoeffWidgetRowStyle(QWidget *widget, QHBoxLayout *layout, QLabel *hLabel, QLabel *hMagLabel, QLabel *hPhaseLabel, QString objectName = "");
private:
    QMap<QString, QString> colorMap;
    static ADMTStyleHelper *pinstance_;
};
} // namespace admt
} // namespace scopy

#endif // ADMTSTYLEHELPER_H