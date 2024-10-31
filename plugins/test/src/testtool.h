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

#ifndef TESTTOOL_H
#define TESTTOOL_H

#include "gui/mapstackedwidget.h"
#include "gui/tooltemplate.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include <scopy-testplugin_export.h>

using namespace scopy;

class SCOPY_TESTPLUGIN_EXPORT TestTool : public QWidget
{
public:
	TestTool(QWidget *parent = nullptr);
	void initData();
public Q_SLOTS:
	void acquireData();

private:
	QWidget *iioWidgetsSettingsHelper();

	int testDataSize = 128;
	double amplitude = 0.5;
	double phase = 0;
	std::vector<double> y2Volt;
	std::vector<double> y1Volt;
	std::vector<double> xTime;

	ToolTemplate *tool;
	QPushButton *btn1, *btn2, *btn3;
	QWidget *createMenu(QWidget *parent);
};
//};
#endif // TESTTOOL_H
