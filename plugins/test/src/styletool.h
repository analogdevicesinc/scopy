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

#ifndef STYLETOOL_H
#define STYLETOOL_H

#include "gui/tooltemplate.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include <scopy-testplugin_export.h>

using namespace scopy;

class SCOPY_TESTPLUGIN_EXPORT StyleTool : public QWidget
{
public:
	StyleTool(QWidget *parent = nullptr);

private:
	QWidget *buildStylePage();
	QWidget *buildPageColors();
};
//};
#endif // STYLETOOL_H
