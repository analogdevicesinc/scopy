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

#ifndef IMUANALYZERINTERFACE_H
#define IMUANALYZERINTERFACE_H

#include "scopy-imuanalyzer_export.h"
#include "scenerenderer.hpp"
#include "bubblelevelrenderer.hpp"
#include "imuanalyzersettings.hpp"
#include "datavisualizer.hpp"

#include <QLineEdit>
#include <QObject>

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <toolbuttons.h>
#include <tooltemplate.h>

#include <menucontrolbutton.h>
#include <printplotmanager.h>
#include <toolbuttons.h>
#include <tooltemplate.h>

#include <iio.h>
#include <iioutil/connectionprovider.h>

#include <hoverwidget.h>
#include <QStackedLayout>
#include <measurementpanel.h>
#include <math.h>
#include <chrono>

namespace scopy {

class SCOPY_IMUANALYZER_EXPORT IMUAnalyzerInterface : public QWidget
{
	Q_OBJECT
public:
	IMUAnalyzerInterface(QString uri, QWidget *parent = nullptr);
	~IMUAnalyzerInterface();

Q_SIGNALS:
	void generateRot(data3P rot);
	void updateValues(data3P rot, data3P pos, float temp);

public:
	void generateRotation();
	void initIIODevice();

private:
	ToolTemplate *m_tool;

	InfoBtn *m_infoBtn;
	RunBtn *m_runBtn;
	MenuControlButton *m_rstView;
	MenuControlButton *m_measureBtn;
	GearBtn *m_gearBtn;

	SceneRenderer *m_sceneRender;
	BubbleLevelRenderer *m_bubbleLevelRenderer;

	ImuAnalyzerSettings *m_settingsPanel;
	DataVisualizer *m_dataV;
	data3P m_rot = {0.0f, 0.0f, 0.0f};
	data3P m_dist = {0.0f, 0.0f, 0.0f};

	std::thread t;
	QString m_uri;

	iio_device *m_device;
};
} // namespace scopy

#endif // IMUANALYZERINTERFACE_H
