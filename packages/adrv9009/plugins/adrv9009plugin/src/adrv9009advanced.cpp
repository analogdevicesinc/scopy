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

#include "adrv9009advanced.h"
#include "advanced/fhmsetupwidget.h"
#include "advanced/paprotectionwidget.h"
#include "advanced/gainsetupwidget.h"
#include "advanced/armgpiowidget.h"
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QLabel>
#include <QSpacerItem>
#include <style.h>
#include <QLoggingCategory>
#include <QScrollArea>

Q_LOGGING_CATEGORY(CAT_ADRV9009_ADVANCED, "ADRV9009_ADVANCED")

using namespace scopy;
using namespace scopy::adrv9009;

Adrv9009Advanced::Adrv9009Advanced(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_ADRV9009_ADVANCED) << "No device provided to ADRV9009 Advanced";
		return;
	}

	const char *device_name = iio_device_get_name(m_device);
	qDebug(CAT_ADRV9009_ADVANCED) << "ADRV9009 Advanced initialized for device:"
				      << (device_name ? device_name : "unknown");

	setupUi();
}

Adrv9009Advanced::~Adrv9009Advanced() {}

void Adrv9009Advanced::setupUi()
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	m_mainLayout->addWidget(m_tool);

	// Add refresh button
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

	// Create main content widget (stacked widget)
	m_centralWidget = new QStackedWidget(this);
	m_tool->addWidgetToCentralContainerHelper(m_centralWidget);

	// Create navigation and content
	createNavigationButtons();
	createContentWidgets();
}

void Adrv9009Advanced::createNavigationButtons()
{
	QButtonGroup *navigationButtons = new QButtonGroup(this);
	navigationButtons->setExclusive(true);

	// Create navigation buttons (13 sections)
	m_clkSettingsBtn = new QPushButton("CLK Settings", this);
	Style::setStyle(m_clkSettingsBtn, style::properties::button::blueGrayButton);
	m_clkSettingsBtn->setCheckable(true);
	m_clkSettingsBtn->setChecked(true); // First button starts selected
	m_clkSettingsBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_calibrationsBtn = new QPushButton("Calibrations", this);
	Style::setStyle(m_calibrationsBtn, style::properties::button::blueGrayButton);
	m_calibrationsBtn->setCheckable(true);
	m_calibrationsBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_txSettingsBtn = new QPushButton("TX Settings", this);
	Style::setStyle(m_txSettingsBtn, style::properties::button::blueGrayButton);
	m_txSettingsBtn->setCheckable(true);
	m_txSettingsBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_rxSettingsBtn = new QPushButton("RX Settings", this);
	Style::setStyle(m_rxSettingsBtn, style::properties::button::blueGrayButton);
	m_rxSettingsBtn->setCheckable(true);
	m_rxSettingsBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_orxSettingsBtn = new QPushButton("ORX Settings", this);
	Style::setStyle(m_orxSettingsBtn, style::properties::button::blueGrayButton);
	m_orxSettingsBtn->setCheckable(true);
	m_orxSettingsBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_fhmSetupBtn = new QPushButton("FHM Setup", this);
	Style::setStyle(m_fhmSetupBtn, style::properties::button::blueGrayButton);
	m_fhmSetupBtn->setCheckable(true);
	m_fhmSetupBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_paProtectionBtn = new QPushButton("PA Protection", this);
	Style::setStyle(m_paProtectionBtn, style::properties::button::blueGrayButton);
	m_paProtectionBtn->setCheckable(true);
	m_paProtectionBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_gainSetupBtn = new QPushButton("GAIN Setup", this);
	Style::setStyle(m_gainSetupBtn, style::properties::button::blueGrayButton);
	m_gainSetupBtn->setCheckable(true);
	m_gainSetupBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_agcSetupBtn = new QPushButton("AGC Setup", this);
	Style::setStyle(m_agcSetupBtn, style::properties::button::blueGrayButton);
	m_agcSetupBtn->setCheckable(true);
	m_agcSetupBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_gpioConfigBtn = new QPushButton("ARM GPIO", this);
	Style::setStyle(m_gpioConfigBtn, style::properties::button::blueGrayButton);
	m_gpioConfigBtn->setCheckable(true);
	m_gpioConfigBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_auxDacBtn = new QPushButton("AUX DAC", this);
	Style::setStyle(m_auxDacBtn, style::properties::button::blueGrayButton);
	m_auxDacBtn->setCheckable(true);
	m_auxDacBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_jesd204SettingsBtn = new QPushButton("JESD204 Settings", this);
	Style::setStyle(m_jesd204SettingsBtn, style::properties::button::blueGrayButton);
	m_jesd204SettingsBtn->setCheckable(true);
	m_jesd204SettingsBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_bistBtn = new QPushButton("BIST", this);
	Style::setStyle(m_bistBtn, style::properties::button::blueGrayButton);
	m_bistBtn->setCheckable(true);
	m_bistBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	// Add buttons to button group
	navigationButtons->addButton(m_clkSettingsBtn);
	navigationButtons->addButton(m_calibrationsBtn);
	navigationButtons->addButton(m_txSettingsBtn);
	navigationButtons->addButton(m_rxSettingsBtn);
	navigationButtons->addButton(m_orxSettingsBtn);
	navigationButtons->addButton(m_fhmSetupBtn);
	navigationButtons->addButton(m_paProtectionBtn);
	navigationButtons->addButton(m_gainSetupBtn);
	navigationButtons->addButton(m_agcSetupBtn);
	navigationButtons->addButton(m_gpioConfigBtn);
	navigationButtons->addButton(m_auxDacBtn);
	navigationButtons->addButton(m_jesd204SettingsBtn);
	navigationButtons->addButton(m_bistBtn);

	// Create main navigation widget with vertical layout for rows
	QWidget *navigationWidget = new QWidget(this);
	navLayout = new QVBoxLayout(navigationWidget);
	navLayout->setMargin(0);
	navLayout->setSpacing(2);

	// Create persistent row widgets that will be reused
	m_firstRow = new QWidget();
	m_firstRowLayout = new QHBoxLayout(m_firstRow);
	m_firstRowLayout->setMargin(0);
	m_firstRowLayout->setSpacing(5);

	m_secondRow = new QWidget();
	m_secondRowLayout = new QHBoxLayout(m_secondRow);
	m_secondRowLayout->setMargin(0);
	m_secondRowLayout->setSpacing(5);

	m_navigationSpacerItem = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);

	// Add persistent row widgets to main navigation layout
	navLayout->addWidget(m_firstRow);
	navLayout->addWidget(m_secondRow);

	updateNavigationButtonsLayout();

	// Add main navigation widget to top container
	m_tool->addWidgetToTopContainerHelper(navigationWidget, TTA_LEFT);

	// Connect button clicks to content switching
	connect(m_clkSettingsBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_clkSettings); });
	connect(m_calibrationsBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_calibrations); });
	connect(m_txSettingsBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_txSettings); });
	connect(m_rxSettingsBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_rxSettings); });
	connect(m_orxSettingsBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_orxSettings); });
	connect(m_fhmSetupBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_fhmSetup); });
	connect(m_paProtectionBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_paProtection); });
	connect(m_gainSetupBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_gainSetup); });
	connect(m_agcSetupBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_agcSetup); });
	connect(m_gpioConfigBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_gpioConfig); });
	connect(m_auxDacBtn, &QPushButton::clicked, this, [this]() { m_centralWidget->setCurrentWidget(m_auxDac); });
	connect(m_jesd204SettingsBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_jesd204Settings); });
	connect(m_bistBtn, &QPushButton::clicked, this, [this]() { m_centralWidget->setCurrentWidget(m_bist); });
}

void Adrv9009Advanced::createContentWidgets()
{
	// Create placeholder widgets for all 13 sections
	m_clkSettings = createPlaceholderWidget("CLK Settings");
	m_calibrations = createPlaceholderWidget("Calibrations");
	m_txSettings = createPlaceholderWidget("TX Settings");
	m_rxSettings = createPlaceholderWidget("RX Settings");
	m_orxSettings = createPlaceholderWidget("ORX Settings");
	m_agcSetup = createPlaceholderWidget("AGC Setup");
	m_fhmSetup = new FhmSetupWidget(m_device, this);
	m_paProtection = new PaProtectionWidget(m_device, this);
	m_gainSetup = new GainSetupWidget(m_device, this);
	m_gpioConfig = new ArmGpioWidget(m_device, this);
	m_auxDac = createPlaceholderWidget("AUX DAC");
	m_jesd204Settings = createPlaceholderWidget("JESD204 Settings");
	m_bist = createPlaceholderWidget("BIST");

	// Add all widgets to stacked widget
	m_centralWidget->addWidget(m_clkSettings);
	m_centralWidget->addWidget(m_calibrations);
	m_centralWidget->addWidget(m_txSettings);
	m_centralWidget->addWidget(m_rxSettings);
	m_centralWidget->addWidget(m_orxSettings);
	m_centralWidget->addWidget(m_fhmSetup);
	m_centralWidget->addWidget(m_paProtection);
	m_centralWidget->addWidget(m_gainSetup);
	m_centralWidget->addWidget(m_agcSetup);
	m_centralWidget->addWidget(m_gpioConfig);
	m_centralWidget->addWidget(m_auxDac);
	m_centralWidget->addWidget(m_jesd204Settings);
	m_centralWidget->addWidget(m_bist);

	// Set first widget as current (CLK Settings)
	m_centralWidget->setCurrentWidget(m_clkSettings);
	if(m_fhmSetup) {
		connect(this, &Adrv9009Advanced::readRequested, m_fhmSetup, &FhmSetupWidget::readRequested);
	}
	if(m_paProtection) {
		connect(this, &Adrv9009Advanced::readRequested, m_paProtection, &PaProtectionWidget::readRequested);
	}
	if(m_gainSetup) {
		connect(this, &Adrv9009Advanced::readRequested, m_gainSetup, &GainSetupWidget::readRequested);
	}
	if(m_gpioConfig) {
		connect(this, &Adrv9009Advanced::readRequested, m_gpioConfig, &ArmGpioWidget::readRequested);
	}
}

void Adrv9009Advanced::updateNavigationButtonsLayout()
{
	// Step 1: Calculate available width (same as before)
	int containerWidth = m_tool->width();
	int refreshButtonWidth = m_refreshButton->width();
	int layoutSpacingAndMargins = 50; // consider the spacing when computin so buttons text will not be cropped
	int availableWidth = containerWidth - layoutSpacingAndMargins - refreshButtonWidth;

	// Create list of all navigation buttons in order (same as before)
	QList<QPushButton *> allButtons = {m_clkSettingsBtn, m_calibrationsBtn, m_txSettingsBtn,   m_rxSettingsBtn,
					   m_orxSettingsBtn, m_fhmSetupBtn,	m_paProtectionBtn, m_gainSetupBtn,
					   m_agcSetupBtn,    m_gpioConfigBtn,	m_auxDacBtn,	   m_jesd204SettingsBtn,
					   m_bistBtn};

	// Step 2: Calculate button distribution (same logic as before)
	QList<QPushButton *> firstRowButtons;
	QList<QPushButton *> remainingButtons;
	int currentWidth = 0;

	for(QPushButton *button : allButtons) {
		int buttonWidth = button->sizeHint().width();

		if(currentWidth + buttonWidth <= availableWidth || firstRowButtons.isEmpty()) {
			firstRowButtons.append(button);
			currentWidth += buttonWidth;
		} else {
			remainingButtons.append(button);
		}
	}

	// Step 3: MOVE buttons to existing layouts (reuse existing widgets!)
	for(QPushButton *button : firstRowButtons) {
		m_firstRowLayout->addWidget(button);
	}

	if(!remainingButtons.isEmpty()) {
		m_tool->topContainer()->setFixedHeight(88);
		m_secondRowLayout->removeItem(m_navigationSpacerItem);
		for(QPushButton *button : remainingButtons) {
			m_secondRowLayout->addWidget(button);
		}
		m_secondRowLayout->addItem(m_navigationSpacerItem);
	} else {
		m_tool->topContainer()->setFixedHeight(44);
	}
}

QWidget *Adrv9009Advanced::createPlaceholderWidget(const QString &sectionName)
{
	QWidget *widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setAlignment(Qt::AlignCenter);

	QLabel *title = new QLabel(sectionName);
	Style::setStyle(title, style::properties::label::menuBig);
	title->setAlignment(Qt::AlignCenter);

	QLabel *subtitle = new QLabel("Advanced Configuration");
	Style::setStyle(subtitle, style::properties::label::menuMedium);
	subtitle->setAlignment(Qt::AlignCenter);

	QLabel *placeholder = new QLabel("Section content coming in next phase...");
	Style::setStyle(placeholder, style::properties::label::menuSmall);
	placeholder->setAlignment(Qt::AlignCenter);

	const char *device_name = iio_device_get_name(m_device);
	QLabel *deviceInfo = new QLabel(QString("Device: %1").arg(device_name ? device_name : "unknown"));
	Style::setStyle(deviceInfo, style::properties::label::menuSmall);
	deviceInfo->setAlignment(Qt::AlignCenter);

	layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));
	layout->addWidget(title);
	layout->addWidget(subtitle);
	layout->addWidget(placeholder);
	layout->addWidget(deviceInfo);
	layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));

	return widget;
}

void Adrv9009Advanced::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	updateNavigationButtonsLayout();
}

#include "moc_adrv9009advanced.cpp"
