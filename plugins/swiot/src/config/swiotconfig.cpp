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

#include <iio.h>

#include <QMessageBox>
#include <QVBoxLayout>

#include <gui/tool_view_builder.hpp>
#include <iioutil/commandqueueprovider.h>

using namespace scopy::swiot;

SwiotConfig::SwiotConfig(struct iio_context *ctx, QWidget *parent)
	: QWidget(parent)
	, m_context(ctx)
	, m_drawArea(nullptr)
	, m_scrollArea(nullptr)
	, m_toolView(nullptr)
	, m_mainView(new QWidget(this))
	, m_statusLabel(new QLabel(this))
	, m_statusContainer(new QWidget(this))
	, m_commandQueue(CommandQueueProvider::GetInstance()->open(ctx))
	, ui(new Ui::ConfigMenu)
{
	m_swiotDevice = iio_context_find_device(ctx, "swiot");
	if(m_swiotDevice == nullptr) {
		qCritical(CAT_SWIOT_CONFIG) << "Critical error: the \"swiot\" device was not found.";
	}

	this->ui->setupUi(this);
	m_configBtn = createConfigBtn();
	this->setLayout(new QVBoxLayout(this));
	this->layout()->addWidget(m_configBtn);

	this->setupToolView(parent);
	this->init();
	this->createPageLayout();
	this->initTutorialProperties();
	QObject::connect(m_configBtn, &QPushButton::pressed, this, &SwiotConfig::onConfigBtnPressed);
}

SwiotConfig::~SwiotConfig()
{
	if(m_commandQueue) {
		CommandQueueProvider::GetInstance()->close(m_context);
		m_commandQueue = nullptr;
		m_context = nullptr;
	}
}

void SwiotConfig::init()
{
	for(int i = 0; i < 4; i++) { // there can only be 4 channels
		auto *channelView = new ConfigChannelView(i);
		auto *configModel = new ConfigModel(m_swiotDevice, i, m_commandQueue);
		auto *configController = new ConfigController(channelView, configModel, i);
		m_controllers.push_back(configController);
		ui->gridLayout->addWidget(channelView, i + 1, 0, 1, 4);

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
}

void SwiotConfig::setDevices(iio_context *ctx)
{
	ssize_t devicesNumber = iio_context_get_devices_count(ctx);
	for(int i = 0; i < devicesNumber; i++) {
		struct iio_device *iioDev = iio_context_get_device(ctx, i);
		if(iioDev) {
			QString deviceName = QString(iio_device_get_name(iioDev));
			m_iioDevices[deviceName] = iioDev;
		}
	}
}

QPushButton *SwiotConfig::createConfigBtn()
{
	auto *configBtn = new QPushButton();
	configBtn->setObjectName(QString::fromUtf8("configBtn"));
	configBtn->setLayoutDirection(Qt::RightToLeft);
	configBtn->setStyleSheet(QString::fromUtf8("QPushButton{\n"
						   "  width: 95px;\n"
						   "  height: 40px;\n"
						   "  font-size: 12px;\n"
						   "  text-align: left;\n"
						   "  font-weight: bold;\n"
						   "  padding-left: 15px;\n"
						   "  padding-right: 15px;\n"
						   "}"));
	configBtn->setProperty("blue_button", QVariant(true));
	QIcon icon = QIcon(":/gui/icons/scopy-default/icons/save.svg");
	configBtn->setIcon(icon);
	configBtn->setIconSize(QSize(13, 13));
	configBtn->setText("Apply");
	return configBtn;
}

void SwiotConfig::onConfigBtnPressed() { Q_EMIT writeModeAttribute("runtime"); }

void SwiotConfig::modeAttributeChanged(std::string mode)
{
	if(mode == "runtime") {
		Q_EMIT configBtnPressed();
	}
}

void SwiotConfig::setupToolView(QWidget *parent)
{
	scopy::gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = false;
	recipe.hasSingleBtn = false;
	recipe.hasPairSettingsBtn = false;
	recipe.hasPrintBtn = false;
	recipe.hasChannels = false;
	recipe.hasHelpBtn = true;

	m_toolView = scopy::gui::ToolViewBuilder(recipe, nullptr, parent).build();

	m_drawArea = new DrawArea(this);
	m_scrollArea = new QScrollArea(this);
}

void SwiotConfig::createPageLayout()
{
	auto scrollWidget = new QWidget(this);
	m_scrollArea->setStyleSheet("background-color: black;");
	scrollWidget->setLayout(new QHBoxLayout(this));
	scrollWidget->layout()->addWidget(this->ui->mainGridContainer);
	scrollWidget->layout()->addWidget(m_drawArea);
	scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_scrollArea->setWidget(scrollWidget);

	m_statusLabel->setText("The system is powered at limited capacity.");
	m_statusLabel->setWordWrap(true);

	m_statusContainer->setLayout(new QHBoxLayout(m_statusContainer));
	m_statusContainer->layout()->setSpacing(0);
	m_statusContainer->layout()->setContentsMargins(0, 0, 0, 0);
	m_statusContainer->setStyleSheet("QWidget{color: #ffc904; background-color: rgba(0, 0, 0, 60); border: 1px "
					 "solid rgba(0, 0, 0, 30); font-size: 11pt}");

	auto exclamationButton = new QPushButton(m_statusContainer);
	exclamationButton->setIcon(QIcon::fromTheme(":/swiot/warning.svg"));
	exclamationButton->setIconSize(QSize(32, 32));
	exclamationButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_statusContainer->layout()->addWidget(exclamationButton);
	m_statusContainer->layout()->addWidget(m_statusLabel);

	m_mainView->setLayout(new QVBoxLayout(m_mainView));
	m_mainView->layout()->setContentsMargins(0, 0, 0, 0);
	m_mainView->layout()->addWidget(m_scrollArea);

	m_toolView->addPlotInfoWidget(m_statusContainer);
	m_toolView->addFixedCentralWidget(m_mainView, 0, 0, 0, 0);
	m_toolView->addTopExtraWidget(m_configBtn);
	this->layout()->addWidget(m_toolView);
}

void SwiotConfig::externalPowerSupply(bool ps)
{
	if(ps) {
		m_statusContainer->hide();
	} else {
		m_statusContainer->show();
		m_statusLabel->show();
	}
}

void SwiotConfig::initTutorialProperties()
{
	m_configBtn->setProperty("tutorial_name", "APPLY_BUTTON");
	m_drawArea->setProperty("tutorial_name", "DRAW_AREA");
}

#include "moc_swiotconfig.cpp"
