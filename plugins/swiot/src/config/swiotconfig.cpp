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

#include "configcontroller.h"
#include "configmodel.h"
#include "src/swiot_logging_categories.h"
#include <QMessageBox>
#include <QVBoxLayout>

#include <iioutil/connectionprovider.h>
#include <gui/stylehelper.h>

using namespace scopy::swiot;

SwiotConfig::SwiotConfig(QString uri, QWidget *parent)
	: QWidget(parent)
	, m_uri(uri)
	, m_ui(new Ui::ConfigMenu)
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
	m_ui->setupUi(this);
	StyleHelper::MenuSmallLabel(m_ui->deviceLabel);
	StyleHelper::MenuSmallLabel(m_ui->functionLabel);

	m_applyBtn = createApplyBtn();
	m_drawArea = new DrawArea(this);
	m_scrollArea = new QScrollArea(this);
}

void SwiotConfig::buildGridLayout()
{
	for(int i = 0; i < NUMBER_OF_CHANNELS; i++) { // there can only be 4 channels
		auto *channelView = new ConfigChannelView(i);
		auto *configModel = new ConfigModel(m_swiotDevice, i, m_commandQueue);
		auto *configController = new ConfigController(channelView, configModel, i);
		m_controllers.push_back(configController);
		m_ui->gridLayout->addWidget(channelView, i + 1, 0, 1, 4);
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
	m_ui->gridLayout->addWidget(m_applyBtn, NUMBER_OF_CHANNELS + 1, 0, 1, 4);
}

void SwiotConfig::createPageLayout()
{
	auto scrollWidget = new QWidget(this);
	scrollWidget->setLayout(new QHBoxLayout(this));
	scrollWidget->layout()->addWidget(this->m_ui->mainGridContainer);
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
	auto *applyBtn = new QPushButton();
	StyleHelper::BlueGrayButton(applyBtn, "applyBtn");
	applyBtn->setCheckable(false);
	applyBtn->setText("Apply");
	return applyBtn;
}

void SwiotConfig::onConfigBtnPressed() { Q_EMIT writeModeAttribute("runtime"); }

void SwiotConfig::modeAttributeChanged(std::string mode)
{
	if(mode == "runtime") {
		Q_EMIT configBtnPressed();
	}
}

#include "moc_swiotconfig.cpp"
