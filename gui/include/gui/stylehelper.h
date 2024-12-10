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

#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include "widgets/hoverwidget.h"
#include <QMap>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QTableWidget>
#include <QSplitter>
#include <QTreeView>
#include <QLineEdit>

#include <scopy-gui_export.h>
#include <utils.h>

#include <QVBoxLayout>
#include <QComboBox>
#include <QListWidget>

// Forward declarations
namespace scopy {
class SmallOnOffSwitch;
class CustomSwitch;
class SpinBoxA;
class MeasurementLabel;
class StatsLabel;
class MeasurementSelectorItem;
class TitleSpinBox;
} // namespace scopy

namespace scopy {
class SCOPY_GUI_EXPORT StyleHelper : public QObject
{
	Q_OBJECT
protected:
	StyleHelper(QObject *parent = nullptr);
	~StyleHelper();

public:
	// singleton
	StyleHelper(StyleHelper &other) = delete;
	void operator=(const StyleHelper &) = delete;
	static StyleHelper *GetInstance();

public:
	static QString getChannelColor(int index);
	static void BasicButton(QPushButton *btn, QString objectName = "");
	static void RefreshButton(QPushButton *btn, QString objectName = "");
	static void BasicSmallButton(QPushButton *btn, QString objectName = "");
	static void CollapseCheckbox(QCheckBox *chk, QString objectName = "");
	static void ColoredCircleCheckbox(QCheckBox *chk, QColor color, QString objectName = "");
	static void ColoredSquareCheckbox(QCheckBox *chk, QColor color, QString objectName = "");
	static void MenuControlButton(QPushButton *btn, QString objectName = "", bool checkable = true);
	static void MenuControlWidget(QWidget *w, QColor color, QString objectName = "");
	static void MenuLargeLabel(QLabel *lbl, QString objectName = "");
	static void MenuHeaderLine(QFrame *line, QPen pen, QString objectName = "");
	static void MenuHeaderWidget(QWidget *w, QString objectName = "");
	static void MenuSectionWidget(QWidget *w, QString objectName = "");
	static void MenuCollapseHeaderLineEdit(QLineEdit *w, QString objectName = "");
	static void BlueIconButton(QPushButton *w, QIcon icon, QString objectName = "");
	static void BackgroundPage(QWidget *w, QString objectName = "");
	static void BackgroundWidget(QWidget *w, QString objectName = "");
	static void MeasurementPanelLabel(MeasurementLabel *w, QString objectName = "");
	static void StatsPanelLabel(StatsLabel *w, QString objectName = "");
	static void MeasurementSelectorItemWidget(QString iconPath, MeasurementSelectorItem *w,
						  QString objectName = "");
	static void HoverWidget(QWidget *w, bool draggable = false, QString objectName = "");
	static void HoverToolTip(QWidget *w, QString info, QString objectName = "");
	static void NoBackgroundIconButton(QPushButton *w, QIcon icon, QString objectName = "");
	static void BrowseButton(QPushButton *btn, QString objectName = "");

private:
	static StyleHelper *pinstance_;
};
} // namespace scopy

#endif // STYLEHELPER_H
