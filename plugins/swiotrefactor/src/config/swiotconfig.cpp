/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#include "swiotconfig.h"
#include "swiot_logging_categories.h"

#include "configcontroller.h"
#include "configmodel.h"

#include <QVBoxLayout>
#include <gui/stylehelper.h>

#include <iioutil/connectionprovider.h>

using namespace scopy::swiotrefactor;

SwiotConfig::SwiotConfig(QString uri, QWidget *parent)
	: QWidget(parent)
	, m_uri(uri)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *layout = new QHBoxLayout(this);
	setLayout(layout);

	// tool template configuration
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	layout->addWidget(m_tool);

	provideDeviceConnection();

	setupUiElements();
	buildGridLayout();
	createPageLayout();
	initTutorialProperties();
	QObject::connect(m_applyBtn, &QPushButton::pressed, this, &SwiotConfig::onConfigBtnPressed);
}

SwiotConfig::~SwiotConfig() { ConnectionProvider::close(m_uri); }

void SwiotConfig::provideDeviceConnection()
{
	Connection *conn = ConnectionProvider::open(m_uri);
	m_context = conn->context();
	m_commandQueue = conn->commandQueue();
	m_swiotDevice = iio_context_find_device(m_context, "swiot");
	if(m_swiotDevice == nullptr) {
		qCritical(CAT_SWIOT_CONFIG) << "Critical error: the \"swiot\" device was not found.";
	}
}

void SwiotConfig::setupUiElements()
{
	m_applyBtn = createApplyBtn();
	m_drawArea = new DrawArea(this);
	m_scrollArea = new QScrollArea(this);
	m_chnlsGrid = new QWidget(this);
	m_chnlsGrid->setLayout(new QVBoxLayout());
	m_chnlsGrid->layout()->setContentsMargins(0, 0, 0, 0);
}

QWidget *SwiotConfig::createGridHeader(QWidget *parent)
{
	QWidget *labels = new QWidget(parent);
	labels->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QHBoxLayout *layout = new QHBoxLayout(labels);
	layout->setContentsMargins(0, 0, 0, 0);

	QLabel *deviceLabel = new QLabel(labels);
	deviceLabel->setText("Device");
	StyleHelper::MenuSmallLabel(deviceLabel);
	deviceLabel->setFixedWidth(DEVICE_COMBO_WIDTH);

	QLabel *functionLabel = new QLabel(labels);
	functionLabel->setText("Function");
	StyleHelper::MenuSmallLabel(functionLabel);
	functionLabel->setFixedWidth(FUNCTION_COMBO_WIDTH);

	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
	layout->addWidget(deviceLabel);
	layout->addWidget(functionLabel);

	return labels;
}

void SwiotConfig::buildGridLayout()
{
	QWidget *header = createGridHeader(m_chnlsGrid);
	m_chnlsGrid->layout()->addWidget(header);

	for(int i = 0; i < NUMBER_OF_CHANNELS; i++) { // there can only be 4 channels
		auto *channelView = new ConfigChannelView(i);
		auto *configModel = new ConfigModel(m_swiotDevice, i, m_commandQueue);
		auto *configController = new ConfigController(channelView, configModel, i);
		m_controllers.push_back(configController);
		m_chnlsGrid->layout()->addWidget(channelView);
		QObject::connect(channelView, &ConfigChannelView::showPath, this,
				 [this, i](int channelIndex, const QString &deviceName) {
					 if(channelIndex == i) {
						 m_drawArea->activateConnection(channelIndex + 1,
										(deviceName == "ad74413r")
											? DrawArea::AD74413R
											: DrawArea::MAX14906);
					 }
				 });

		QObject::connect(channelView, &ConfigChannelView::hidePaths, this,
				 [this]() { m_drawArea->deactivateConnections(); });

		QObject::connect(configController, &ConfigController::clearDrawArea, this,
				 [this]() { m_drawArea->deactivateConnections(); });
	}
	// The apply button will always be at the end of the channel widgets
	m_chnlsGrid->layout()->addWidget(m_applyBtn);
	m_chnlsGrid->layout()->addItem(new QSpacerItem(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding));
}

void SwiotConfig::createPageLayout()
{
	auto scrollWidget = new QWidget(this);
	scrollWidget->setLayout(new QHBoxLayout(this));
	scrollWidget->layout()->addWidget(m_chnlsGrid);
	scrollWidget->layout()->addWidget(m_drawArea);
	scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	scrollWidget->layout()->setSpacing(40);
	m_scrollArea->setWidget(scrollWidget);

	m_tool->addWidgetToCentralContainerHelper(m_scrollArea);
}

void SwiotConfig::initTutorialProperties()
{
	m_applyBtn->setProperty("tutorial_name", "APPLY_BUTTON");
	m_drawArea->setProperty("tutorial_name", "DRAW_AREA");
}

QPushButton *SwiotConfig::createApplyBtn()
{
	auto *applyBtn = new QPushButton(this);
	StyleHelper::BlueGrayButton(applyBtn, "applyBtn");
	applyBtn->setCheckable(false);
	applyBtn->setText("Apply");
	return applyBtn;
}

void SwiotConfig::onConfigBtnPressed() { Q_EMIT writeModeAttribute("runtime"); }

#include "moc_swiotconfig.cpp"
