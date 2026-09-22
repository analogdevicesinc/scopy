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

#include "imuanalyzerinterface.hpp"

#include <pluginbase/preferences.h>

#include <QTimer>
#include <qcoro/qcorotask.h>

#include <component/context.h>
#include <component/device.h>
#include <component/channel.h>
#include <component/attribute.h>
#include <component/navigation.h>

using namespace scopy;

static constexpr int IMU_POLL_INTERVAL_MS = 30;

Q_DECLARE_METATYPE(data3P)

IMUAnalyzerInterface::IMUAnalyzerInterface(component::Device *device, QWidget *parent)
	: QWidget{parent}
	, m_sceneRender(nullptr)
	, m_rstView(nullptr)
{
	qRegisterMetaType<data3P>("data3P");

	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	setLayout(lay);

	m_device = device;

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, [this]() {
		if(m_cycleInFlight) {
			return;
		}
		m_activeCycle = readCycle();
	});

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->leftContainer()->setVisible(false);
	m_tool->rightContainer()->setVisible(true);
	m_tool->bottomContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->setRightContainerWidth(300);
	m_tool->centralContainer()->layout()->setSpacing(10);

	lay->addWidget(m_tool);

	m_dataV = new DataVisualizer();
	m_tool->addWidgetToCentralContainerHelper(m_dataV);
	m_dataV->hide();

	connect(this, &IMUAnalyzerInterface::updateValues, m_dataV, &DataVisualizer::updateValues);

	m_infoBtn = new InfoBtn(this, false);
	m_tool->addWidgetToTopContainerHelper(m_infoBtn, TTA_LEFT);

	m_runBtn = new RunBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);

	bool useOpenGL = Preferences::get("general_use_opengl").toBool();

	QTabWidget *tabWidget = new QTabWidget(this);
	m_tool->addWidgetToCentralContainerHelper(tabWidget);

	if(useOpenGL) {
		m_sceneRender = new SceneRenderer();
		tabWidget->addTab(m_sceneRender, "3D View");

		m_rstView = new MenuControlButton(this);
		m_rstView->setName("Reset View");
		m_rstView->checkBox()->setVisible(false);
		m_rstView->button()->setVisible(false);
		m_rstView->setCheckable(false);
		m_tool->addWidgetToBottomContainerHelper(m_rstView, TTA_LEFT);

		connect(m_rstView, &QPushButton::clicked, m_sceneRender, &SceneRenderer::resetView);
		connect(this, &IMUAnalyzerInterface::generateRot, m_sceneRender, &SceneRenderer::setRot);
		connect(tabWidget, &QTabWidget::currentChanged, this,
			[=, this](int index) { m_rstView->setVisible(index == 0); });
	}

	m_measureBtn = new MenuControlButton(this);
	m_measureBtn->setName("Measure");
	m_measureBtn->checkBox()->setVisible(false);
	m_measureBtn->button()->setVisible(false);
	m_measureBtn->setCheckable(true);
	m_measureBtn->setChecked(false);
	m_tool->addWidgetToBottomContainerHelper(m_measureBtn, TTA_RIGHT);

	connect(m_measureBtn, &QPushButton::toggled, this, [=, this](bool toggled) {
		if(toggled)
			m_dataV->show();
		else
			m_dataV->hide();
	});

	connect(m_runBtn, &QPushButton::toggled, [=, this](bool toggled) {
		if(toggled) {
			QCoro::waitFor(initGains());
			m_timer->start(IMU_POLL_INTERVAL_MS);
		} else {
			m_timer->stop();
		}
		Q_EMIT runBtnPressed(toggled);
	});

	m_bubbleLevelRenderer = new BubbleLevelRenderer(tabWidget);
	tabWidget->addTab(m_bubbleLevelRenderer, "2D View");

	connect(this, &IMUAnalyzerInterface::updateValues, m_bubbleLevelRenderer, &BubbleLevelRenderer::setRot);

	m_gearBtn = new GearBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_gearBtn, TTA_RIGHT);

	m_gearBtn->setChecked(true);

	QString key = "Settings";
	m_settingsPanel = new ImuAnalyzerSettings(m_sceneRender, m_bubbleLevelRenderer, m_device);

	QScrollArea *scrollArea = new QScrollArea;
	scrollArea->setWidget(m_settingsPanel);
	scrollArea->setWidgetResizable(true);

	m_tool->rightStack()->add(key, scrollArea);

	connect(m_gearBtn, &GearBtn::toggled, this, [=, this](bool toggled) {
		if(toggled)
			m_tool->requestMenu(key);

		m_tool->openRightContainerHelper(toggled);
	});
}

IMUAnalyzerInterface::~IMUAnalyzerInterface()
{
	if(m_timer) {
		m_timer->stop();
	}
	if(m_activeCycle) {
		QCoro::waitFor(m_activeCycle.value());
		m_activeCycle.reset();
	}
}

void IMUAnalyzerInterface::runToggled(bool toggled) { m_runBtn->setChecked(toggled); }

QCoro::Task<void> IMUAnalyzerInterface::initGains()
{
	m_accelX = component::channelById(m_device, "accel_x", false);
	m_accelY = component::channelById(m_device, "accel_y", false);
	m_accelZ = component::channelById(m_device, "accel_z", false);
	m_temp = component::channelById(m_device, "temp0", false);
	m_hasTemp = (m_temp != nullptr);

	component::Attribute *samplingAttr = component::attributeByName(m_device, "sampling_frequency");
	if(samplingAttr && samplingAttr->readCapability()) {
		co_await samplingAttr->readCapability()->readAsync();
		m_samplingFreq = samplingAttr->cachedValue().toDouble();
	}

	if(m_accelX) {
		component::Attribute *a = component::attributeByName(m_accelX, "scale");
		if(a && a->readCapability()) {
			co_await a->readCapability()->readAsync();
			m_gainX = a->cachedValue().toDouble();
		}
	}
	if(m_accelY) {
		component::Attribute *a = component::attributeByName(m_accelY, "scale");
		if(a && a->readCapability()) {
			co_await a->readCapability()->readAsync();
			m_gainY = a->cachedValue().toDouble();
		}
	}
	if(m_accelZ) {
		component::Attribute *a = component::attributeByName(m_accelZ, "scale");
		if(a && a->readCapability()) {
			co_await a->readCapability()->readAsync();
			m_gainZ = a->cachedValue().toDouble();
		}
	}
	if(m_hasTemp) {
		component::Attribute *scaleAttr = component::attributeByName(m_temp, "scale");
		if(scaleAttr && scaleAttr->readCapability()) {
			co_await scaleAttr->readCapability()->readAsync();
			m_tempGain = scaleAttr->cachedValue().toDouble();
		}
		component::Attribute *offsetAttr = component::attributeByName(m_temp, "offset");
		if(offsetAttr && offsetAttr->readCapability()) {
			co_await offsetAttr->readCapability()->readAsync();
			m_tempOffset = offsetAttr->cachedValue().toDouble();
		}
	}
}

QCoro::Task<void> IMUAnalyzerInterface::readCycle()
{
	m_cycleInFlight = true;

	double linearAccX = 0, linearAccY = 0, linearAccZ = 0;
	double temp = 0;

	if(m_accelX) {
		component::Attribute *a = component::attributeByName(m_accelX, "raw");
		if(a && a->readCapability()) {
			co_await a->readCapability()->readAsync();
			linearAccX = a->cachedValue().toDouble();
		}
	}
	if(m_accelY) {
		component::Attribute *a = component::attributeByName(m_accelY, "raw");
		if(a && a->readCapability()) {
			co_await a->readCapability()->readAsync();
			linearAccY = a->cachedValue().toDouble();
		}
	}
	if(m_accelZ) {
		component::Attribute *a = component::attributeByName(m_accelZ, "raw");
		if(a && a->readCapability()) {
			co_await a->readCapability()->readAsync();
			linearAccZ = a->cachedValue().toDouble();
		}
	}

	m_dist.dataX = float(linearAccX * m_gainX);
	m_dist.dataY = float(linearAccY * m_gainY);
	m_dist.dataZ = float(linearAccZ * m_gainZ);

	if(m_hasTemp) {
		component::Attribute *a = component::attributeByName(m_temp, "raw");
		if(a && a->readCapability()) {
			co_await a->readCapability()->readAsync();
			temp = a->cachedValue().toDouble();
		}
		temp = temp * m_tempGain - m_tempOffset;
	}

	m_rot.dataX =
		atan2(-m_dist.dataX, sqrt(m_dist.dataY * m_dist.dataY + m_dist.dataZ * m_dist.dataZ)) * 180 / 3.14f;
	m_rot.dataY = atan2(m_dist.dataY, m_dist.dataZ) * 180 / 3.14f;
	m_rot.dataZ = 0;

	Q_EMIT generateRot(m_rot);
	Q_EMIT updateValues(m_rot, m_dist, float(temp));

	m_cycleInFlight = false;
}

#include "moc_imuanalyzerinterface.cpp"
