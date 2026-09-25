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

#include <optional>

#include <qcoro/qcorotask.h>
#include <QTimer>

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

#include <hoverwidget.h>
#include <QStackedLayout>
#include <measurementpanel.h>
#include <math.h>

namespace scopy::component {
class Device;
class Channel;
} // namespace scopy::component

namespace scopy {

class SCOPY_IMUANALYZER_EXPORT IMUAnalyzerInterface : public QWidget
{
	Q_OBJECT
public:
	IMUAnalyzerInterface(component::Device *device, QWidget *parent = nullptr);
	~IMUAnalyzerInterface();

public Q_SLOTS:
	void runToggled(bool toggled);

Q_SIGNALS:
	void runBtnPressed(bool toggled);
	void generateRot(data3P rot);
	void updateValues(data3P rot, data3P pos, float temp);

private:
	QCoro::Task<void> initGains();
	QCoro::Task<void> readCycle();

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

	component::Device *m_device = nullptr;
	component::Channel *m_accelX = nullptr, *m_accelY = nullptr, *m_accelZ = nullptr, *m_temp = nullptr;
	double m_gainX = 0, m_gainY = 0, m_gainZ = 0, m_tempGain = 0, m_tempOffset = 0, m_samplingFreq = 0;
	bool m_hasTemp = false;

	QTimer *m_timer = nullptr;
	bool m_cycleInFlight = false;
	std::optional<QCoro::Task<void>> m_activeCycle;
};
} // namespace scopy

#endif // IMUANALYZERINTERFACE_H
