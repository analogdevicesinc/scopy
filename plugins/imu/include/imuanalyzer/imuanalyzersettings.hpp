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
 */

#ifndef IMUANALYZERSETTINGS_H
#define IMUANALYZERSETTINGS_H

#include <QWidget>
#include <QLineEdit>
#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include <edittextmenuheader.h>
#include <menucombo.h>
#include <menuonoffswitch.h>
#include <menucontrolbutton.h>
#include <menuheader.h>
#include <menucollapsesection.h>
#include <menusectionwidget.h>
#include "bubblelevelrenderer.hpp"
#include "scenerenderer.hpp"
#include "menuspinbox.h"

#include <QColorDialog>
#include <QPalette>

#include "scopy-imuanalyzer_export.h"
#include "iio.h"
#include "iio-widgets/iiowidget.h"
#include "iio-widgets/iiowidgetbuilder.h"
#include "iio-widgets/iiowidgetdata.h"

namespace scopy {

class SCOPY_IMUANALYZER_EXPORT ImuAnalyzerSettings : public QWidget
{
	Q_OBJECT
public:
	explicit ImuAnalyzerSettings(SceneRenderer *scRend, BubbleLevelRenderer *blRend, iio_device *device,
				     QWidget *parent = nullptr);

signals:
	void updateCubeColor(QColor color);
	void updatePlaneColor(QColor color);
	void updateDisplayPoints(QString displayP);

private:
	iio_device *m_device;
};
} // namespace scopy
#endif // IMUANALYZERSETTINGS_H
