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
 *
 */

#include "ad9371advanced.h"

#include <QFutureWatcher>
#include <QScrollArea>
#include <QtConcurrent>
#include <style.h>
#include <toolbuttons.h>
#include <QLoggingCategory>
#include <pluginbase/preferences.h>

Q_LOGGING_CATEGORY(CAT_AD9371_ADVANCED, "AD9371_ADVANCED")

using namespace scopy;
using namespace ad9371;

AD9371Advanced::AD9371Advanced(iio_context *ctx, IIOWidgetGroup *group, bool has_dpd, QWidget *parent)
	: m_ctx(ctx)
	, m_group(group)
	, m_hasDpd(has_dpd)
	, QWidget{parent}
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	m_mainLayout->addWidget(m_tool);

	m_refreshButton = new AnimatedRefreshBtn(false, this);
	m_tool->addWidgetToTopContainerHelper(m_refreshButton, TTA_RIGHT);

	connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
		m_refreshButton->startAnimation();

		QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
		connect(
			watcher, &QFutureWatcher<void>::finished, this,
			[this, watcher]() {
				m_refreshButton->stopAnimation();
				watcher->deleteLater();
			},
			Qt::QueuedConnection);

		QFuture<void> future = QtConcurrent::run([this]() { Q_EMIT readRequested(); });

		watcher->setFuture(future);
	});

	m_isToolInitialized = false;

	QStackedWidget *centralWidget = new QStackedWidget(this);
	m_tool->addWidgetToCentralContainerHelper(centralWidget);

	QButtonGroup *navigationButtons = new QButtonGroup(this);
	navigationButtons->setExclusive(true);

	if(m_ctx != nullptr) {
		iio_device *phyDev = iio_context_find_device(m_ctx, "ad9371-phy");
		if(phyDev == nullptr) {
			qWarning(CAT_AD9371_ADVANCED) << "No ad9371-phy device found in context!";
			return;
		}

		m_phyDevice = phyDev;
		m_centralWidget = centralWidget;

		// Create navigation buttons
		m_clkSettingsBtn = new QPushButton("CLK Settings", this);
		Style::setStyle(m_clkSettingsBtn, style::properties::button::blueGrayButton);
		m_clkSettingsBtn->setCheckable(true);
		m_clkSettingsBtn->setChecked(true);

		m_calibrationsBtn = new QPushButton("Calibrations", this);
		Style::setStyle(m_calibrationsBtn, style::properties::button::blueGrayButton);
		m_calibrationsBtn->setCheckable(true);

		m_txSettingsBtn = new QPushButton("TX Settings", this);
		Style::setStyle(m_txSettingsBtn, style::properties::button::blueGrayButton);
		m_txSettingsBtn->setCheckable(true);

		m_rxSettingsBtn = new QPushButton("RX Settings", this);
		Style::setStyle(m_rxSettingsBtn, style::properties::button::blueGrayButton);
		m_rxSettingsBtn->setCheckable(true);

		m_obsSettingsBtn = new QPushButton("OBS Settings", this);
		Style::setStyle(m_obsSettingsBtn, style::properties::button::blueGrayButton);
		m_obsSettingsBtn->setCheckable(true);

		m_gainSetupBtn = new QPushButton("GAIN Setup", this);
		Style::setStyle(m_gainSetupBtn, style::properties::button::blueGrayButton);
		m_gainSetupBtn->setCheckable(true);

		m_agcSetupBtn = new QPushButton("AGC Setup", this);
		Style::setStyle(m_agcSetupBtn, style::properties::button::blueGrayButton);
		m_agcSetupBtn->setCheckable(true);

		m_armGpioBtn = new QPushButton("ARM GPIO", this);
		Style::setStyle(m_armGpioBtn, style::properties::button::blueGrayButton);
		m_armGpioBtn->setCheckable(true);

		m_gpioBtn = new QPushButton("GPIO", this);
		Style::setStyle(m_gpioBtn, style::properties::button::blueGrayButton);
		m_gpioBtn->setCheckable(true);

		m_auxDacBtn = new QPushButton("AUX DAC", this);
		Style::setStyle(m_auxDacBtn, style::properties::button::blueGrayButton);
		m_auxDacBtn->setCheckable(true);

		m_jesdFramerBtn = new QPushButton("JESD Framer", this);
		Style::setStyle(m_jesdFramerBtn, style::properties::button::blueGrayButton);
		m_jesdFramerBtn->setCheckable(true);

		m_jesdDeframerBtn = new QPushButton("JESD Deframer", this);
		Style::setStyle(m_jesdDeframerBtn, style::properties::button::blueGrayButton);
		m_jesdDeframerBtn->setCheckable(true);

		m_bistBtn = new QPushButton("BIST", this);
		Style::setStyle(m_bistBtn, style::properties::button::blueGrayButton);
		m_bistBtn->setCheckable(true);

		navigationButtons->addButton(m_clkSettingsBtn);
		navigationButtons->addButton(m_calibrationsBtn);
		navigationButtons->addButton(m_txSettingsBtn);
		navigationButtons->addButton(m_rxSettingsBtn);
		navigationButtons->addButton(m_obsSettingsBtn);
		navigationButtons->addButton(m_gainSetupBtn);
		navigationButtons->addButton(m_agcSetupBtn);
		navigationButtons->addButton(m_armGpioBtn);
		navigationButtons->addButton(m_gpioBtn);
		navigationButtons->addButton(m_auxDacBtn);
		navigationButtons->addButton(m_jesdFramerBtn);
		navigationButtons->addButton(m_jesdDeframerBtn);
		navigationButtons->addButton(m_bistBtn);

		m_tool->addWidgetToTopContainerHelper(m_clkSettingsBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_calibrationsBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_txSettingsBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_rxSettingsBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_obsSettingsBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_gainSetupBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_agcSetupBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_armGpioBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_gpioBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_auxDacBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_jesdFramerBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_jesdDeframerBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_bistBtn, TTA_LEFT);

		if(m_hasDpd) {
			m_dpdSettingsBtn = new QPushButton("DPD Settings", this);
			Style::setStyle(m_dpdSettingsBtn, style::properties::button::blueGrayButton);
			m_dpdSettingsBtn->setCheckable(true);

			m_clgcSettingsBtn = new QPushButton("CLGC Settings", this);
			Style::setStyle(m_clgcSettingsBtn, style::properties::button::blueGrayButton);
			m_clgcSettingsBtn->setCheckable(true);

			m_vswrSettingsBtn = new QPushButton("VSWR Settings", this);
			Style::setStyle(m_vswrSettingsBtn, style::properties::button::blueGrayButton);
			m_vswrSettingsBtn->setCheckable(true);

			navigationButtons->addButton(m_dpdSettingsBtn);
			navigationButtons->addButton(m_clgcSettingsBtn);
			navigationButtons->addButton(m_vswrSettingsBtn);

			m_tool->addWidgetToTopContainerHelper(m_dpdSettingsBtn, TTA_LEFT);
			m_tool->addWidgetToTopContainerHelper(m_clgcSettingsBtn, TTA_LEFT);
			m_tool->addWidgetToTopContainerHelper(m_vswrSettingsBtn, TTA_LEFT);
		}

		bool useLazyLoading = scopy::Preferences::get("iiowidgets_use_lazy_loading").toBool();
		if(!useLazyLoading) {
			init();
		}
	}
}

AD9371Advanced::~AD9371Advanced() {}

void AD9371Advanced::showEvent(QShowEvent *event)
{
	if(!m_isToolInitialized) {
		bool useLazyLoading = scopy::Preferences::get("iiowidgets_use_lazy_loading").toBool();
		if(useLazyLoading) {
			init();
		}
	}
	QWidget::showEvent(event);
}

void AD9371Advanced::init()
{
	// Helper to wrap a widget in a scroll area and add to the stack
	auto addPage = [this](QWidget *page) -> QWidget * {
		QScrollArea *scroll = new QScrollArea(m_centralWidget);
		scroll->setWidgetResizable(true);
		scroll->setWidget(page);
		m_centralWidget->addWidget(scroll);
		return scroll;
	};

	// CLK Settings
	m_clkSettings = new ClkSettingsWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *clkPage = addPage(m_clkSettings);
	connect(this, &AD9371Advanced::readRequested, m_clkSettings, &ClkSettingsWidget::readRequested);
	connect(m_clkSettingsBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(clkPage); });

	// Calibrations
	m_calibrations = new CalibrationsWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *calPage = addPage(m_calibrations);
	connect(this, &AD9371Advanced::readRequested, m_calibrations, &CalibrationsWidget::readRequested);
	connect(m_calibrationsBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(calPage); });

	// TX Settings
	m_txSettings = new TxSettingsWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *txPage = addPage(m_txSettings);
	connect(this, &AD9371Advanced::readRequested, m_txSettings, &TxSettingsWidget::readRequested);
	connect(m_txSettingsBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(txPage); });

	// RX Settings
	m_rxSettings = new RxSettingsWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *rxPage = addPage(m_rxSettings);
	connect(this, &AD9371Advanced::readRequested, m_rxSettings, &RxSettingsWidget::readRequested);
	connect(m_rxSettingsBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(rxPage); });

	// OBS Settings
	m_obsSettings = new ObsSettingsWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *obsPage = addPage(m_obsSettings);
	connect(this, &AD9371Advanced::readRequested, m_obsSettings, &ObsSettingsWidget::readRequested);
	connect(m_obsSettingsBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(obsPage); });

	// GAIN Setup
	m_gainSetup = new GainSetupWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *gainPage = addPage(m_gainSetup);
	connect(this, &AD9371Advanced::readRequested, m_gainSetup, &GainSetupWidget::readRequested);
	connect(m_gainSetupBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(gainPage); });

	// AGC Setup
	m_agcSetup = new AgcSetupWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *agcPage = addPage(m_agcSetup);
	connect(this, &AD9371Advanced::readRequested, m_agcSetup, &AgcSetupWidget::readRequested);
	connect(m_agcSetupBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(agcPage); });

	// ARM GPIO
	m_armGpio = new ArmGpioWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *armPage = addPage(m_armGpio);
	connect(this, &AD9371Advanced::readRequested, m_armGpio, &ArmGpioWidget::readRequested);
	connect(m_armGpioBtn, &QPushButton::clicked, this, [=, this]() { m_centralWidget->setCurrentWidget(armPage); });

	// GPIO
	m_gpio = new GpioWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *gpioPage = addPage(m_gpio);
	connect(this, &AD9371Advanced::readRequested, m_gpio, &GpioWidget::readRequested);
	connect(m_gpioBtn, &QPushButton::clicked, this, [=, this]() { m_centralWidget->setCurrentWidget(gpioPage); });

	// AUX DAC
	m_auxDac = new AuxDacWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *auxPage = addPage(m_auxDac);
	connect(this, &AD9371Advanced::readRequested, m_auxDac, &AuxDacWidget::readRequested);
	connect(m_auxDacBtn, &QPushButton::clicked, this, [=, this]() { m_centralWidget->setCurrentWidget(auxPage); });

	// JESD Framer
	m_jesdFramer = new JesdFramerWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *framerPage = addPage(m_jesdFramer);
	connect(this, &AD9371Advanced::readRequested, m_jesdFramer, &JesdFramerWidget::readRequested);
	connect(m_jesdFramerBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(framerPage); });

	// JESD Deframer
	m_jesdDeframer = new JesdDeframerWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *deframerPage = addPage(m_jesdDeframer);
	connect(this, &AD9371Advanced::readRequested, m_jesdDeframer, &JesdDeframerWidget::readRequested);
	connect(m_jesdDeframerBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(deframerPage); });

	// BIST
	m_bist = new Ad9371BistWidget(m_phyDevice, m_group, m_centralWidget);
	QWidget *bistPage = addPage(m_bist);
	connect(this, &AD9371Advanced::readRequested, m_bist, &Ad9371BistWidget::readRequested);
	connect(m_bistBtn, &QPushButton::clicked, this, [=, this]() { m_centralWidget->setCurrentWidget(bistPage); });

	// Conditional DPD/CLGC/VSWR pages
	if(m_hasDpd) {
		m_dpdSettings = new DpdSettingsWidget(m_phyDevice, m_group, m_centralWidget);
		QWidget *dpdPage = addPage(m_dpdSettings);
		connect(this, &AD9371Advanced::readRequested, m_dpdSettings, &DpdSettingsWidget::readRequested);
		connect(m_dpdSettingsBtn, &QPushButton::clicked, this,
			[=, this]() { m_centralWidget->setCurrentWidget(dpdPage); });

		m_clgcSettings = new ClgcSettingsWidget(m_phyDevice, m_group, m_centralWidget);
		QWidget *clgcPage = addPage(m_clgcSettings);
		connect(this, &AD9371Advanced::readRequested, m_clgcSettings, &ClgcSettingsWidget::readRequested);
		connect(m_clgcSettingsBtn, &QPushButton::clicked, this,
			[=, this]() { m_centralWidget->setCurrentWidget(clgcPage); });

		m_vswrSettings = new VswrSettingsWidget(m_phyDevice, m_group, m_centralWidget);
		QWidget *vswrPage = addPage(m_vswrSettings);
		connect(this, &AD9371Advanced::readRequested, m_vswrSettings, &VswrSettingsWidget::readRequested);
		connect(m_vswrSettingsBtn, &QPushButton::clicked, this,
			[=, this]() { m_centralWidget->setCurrentWidget(vswrPage); });
	}

	m_isToolInitialized = true;
}
