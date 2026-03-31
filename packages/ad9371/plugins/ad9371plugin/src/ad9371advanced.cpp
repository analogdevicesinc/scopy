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

#include "ad9371advanced.h"
#include "advanced/clksettingswidget.h"
#include "advanced/calibrationwidget.h"
#include "advanced/txsettingswidget.h"
#include "advanced/rxsettingswidget.h"
#include "advanced/obssettingswidget.h"
#include "advanced/gainsetupwidget.h"
#include "advanced/agcsetupwidget.h"
#include "advanced/armgpiowidget.h"
#include "advanced/gpiowidget.h"
#include "advanced/auxdacwidget.h"
#include "advanced/jesdframerwidget.h"
#include "advanced/jesddeframerwidget.h"
#include "advanced/bistwidget.h"
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QLabel>
#include <QSpacerItem>
#include <style.h>
#include <QLoggingCategory>
#include <QScrollArea>

Q_LOGGING_CATEGORY(CAT_AD9371_ADVANCED, "AD9371_ADVANCED")

using namespace scopy;
using namespace scopy::ad9371;

Ad9371Advanced::Ad9371Advanced(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_ADVANCED) << "No device provided to AD9371 Advanced";
		return;
	}

	const char *device_name = iio_device_get_name(m_device);
	qDebug(CAT_AD9371_ADVANCED) << "AD9371 Advanced initialized for device:"
				    << (device_name ? device_name : "unknown");

	setupUi();
}

Ad9371Advanced::~Ad9371Advanced() {}

void Ad9371Advanced::setupUi()
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

void Ad9371Advanced::createNavigationButtons()
{
	QButtonGroup *navigationButtons = new QButtonGroup(this);
	navigationButtons->setExclusive(true);

	// Create navigation buttons (16 sections)
	m_clkSettingsBtn = new QPushButton("CLK Settings", this);
	Style::setStyle(m_clkSettingsBtn, style::properties::button::blueGrayButton);
	m_clkSettingsBtn->setCheckable(true);
	m_clkSettingsBtn->setChecked(true);
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

	m_obsSettingsBtn = new QPushButton("OBS Settings", this);
	Style::setStyle(m_obsSettingsBtn, style::properties::button::blueGrayButton);
	m_obsSettingsBtn->setCheckable(true);
	m_obsSettingsBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_gainSetupBtn = new QPushButton("GAIN Setup", this);
	Style::setStyle(m_gainSetupBtn, style::properties::button::blueGrayButton);
	m_gainSetupBtn->setCheckable(true);
	m_gainSetupBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_agcSetupBtn = new QPushButton("AGC Setup", this);
	Style::setStyle(m_agcSetupBtn, style::properties::button::blueGrayButton);
	m_agcSetupBtn->setCheckable(true);
	m_agcSetupBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_armGpioBtn = new QPushButton("ARM GPIO", this);
	Style::setStyle(m_armGpioBtn, style::properties::button::blueGrayButton);
	m_armGpioBtn->setCheckable(true);
	m_armGpioBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_gpioBtn = new QPushButton("GPIO", this);
	Style::setStyle(m_gpioBtn, style::properties::button::blueGrayButton);
	m_gpioBtn->setCheckable(true);
	m_gpioBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_auxDacBtn = new QPushButton("AUX DAC", this);
	Style::setStyle(m_auxDacBtn, style::properties::button::blueGrayButton);
	m_auxDacBtn->setCheckable(true);
	m_auxDacBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_jesdFramerBtn = new QPushButton("JESD Framer", this);
	Style::setStyle(m_jesdFramerBtn, style::properties::button::blueGrayButton);
	m_jesdFramerBtn->setCheckable(true);
	m_jesdFramerBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_jesdDeframerBtn = new QPushButton("JESD Deframer", this);
	Style::setStyle(m_jesdDeframerBtn, style::properties::button::blueGrayButton);
	m_jesdDeframerBtn->setCheckable(true);
	m_jesdDeframerBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_bistBtn = new QPushButton("BIST", this);
	Style::setStyle(m_bistBtn, style::properties::button::blueGrayButton);
	m_bistBtn->setCheckable(true);
	m_bistBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	// Add buttons to button group
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

	// Create main navigation widget with vertical layout for rows
	QWidget *navigationWidget = new QWidget(this);
	navLayout = new QVBoxLayout(navigationWidget);
	navLayout->setMargin(0);
	navLayout->setSpacing(2);

	m_firstRow = new QWidget();
	m_firstRowLayout = new QHBoxLayout(m_firstRow);
	m_firstRowLayout->setMargin(0);
	m_firstRowLayout->setSpacing(5);

	m_secondRow = new QWidget();
	m_secondRowLayout = new QHBoxLayout(m_secondRow);
	m_secondRowLayout->setMargin(0);
	m_secondRowLayout->setSpacing(5);

	m_navigationSpacerItem = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);

	navLayout->addWidget(m_firstRow);
	navLayout->addWidget(m_secondRow);

	updateNavigationButtonsLayout();

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
	connect(m_obsSettingsBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_obsSettings); });
	connect(m_gainSetupBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_gainSetup); });
	connect(m_agcSetupBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_agcSetup); });
	connect(m_armGpioBtn, &QPushButton::clicked, this, [this]() { m_centralWidget->setCurrentWidget(m_armGpio); });
	connect(m_gpioBtn, &QPushButton::clicked, this, [this]() { m_centralWidget->setCurrentWidget(m_gpio); });
	connect(m_auxDacBtn, &QPushButton::clicked, this, [this]() { m_centralWidget->setCurrentWidget(m_auxDac); });
	connect(m_jesdFramerBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_jesdFramer); });
	connect(m_jesdDeframerBtn, &QPushButton::clicked, this,
		[this]() { m_centralWidget->setCurrentWidget(m_jesdDeframer); });
	connect(m_bistBtn, &QPushButton::clicked, this, [this]() { m_centralWidget->setCurrentWidget(m_bist); });
}


void Ad9371Advanced::createContentWidgets()
{
	m_clkSettings = new ClkSettingsWidget(m_device, m_widgetGroup, this);
	m_calibrations = new CalibrationWidget(m_device, m_widgetGroup, this);
	m_txSettings = new TxSettingsWidget(m_device, m_widgetGroup, this);
	m_rxSettings = new RxSettingsWidget(m_device, m_widgetGroup, this);
	m_obsSettings = new ObsSettingsWidget(m_device, m_widgetGroup, this);
	m_gainSetup = new GainSetupWidget(m_device, m_widgetGroup, this);
	m_agcSetup = new AgcSetupWidget(m_device, m_widgetGroup, this);
	m_armGpio = new ArmGpioWidget(m_device, m_widgetGroup, this);
	m_gpio = new GpioWidget(m_device, m_widgetGroup, this);
	m_auxDac = new AuxDacWidget(m_device, m_widgetGroup, this);
	m_jesdFramer = new JesdFramerWidget(m_device, m_widgetGroup, this);
	m_jesdDeframer = new JesdDeframerWidget(m_device, m_widgetGroup, this);
	m_bist = new BistWidget(m_device, m_widgetGroup, this);
	// Connect refresh signals
	if(m_clkSettings)
		connect(this, &Ad9371Advanced::readRequested, m_clkSettings, &ClkSettingsWidget::readRequested);
	if(m_calibrations)
		connect(this, &Ad9371Advanced::readRequested, m_calibrations, &CalibrationWidget::readRequested);
	if(m_txSettings)
		connect(this, &Ad9371Advanced::readRequested, m_txSettings, &TxSettingsWidget::readRequested);
	if(m_rxSettings)
		connect(this, &Ad9371Advanced::readRequested, m_rxSettings, &RxSettingsWidget::readRequested);
	if(m_obsSettings)
		connect(this, &Ad9371Advanced::readRequested, m_obsSettings, &ObsSettingsWidget::readRequested);
	if(m_gainSetup)
		connect(this, &Ad9371Advanced::readRequested, m_gainSetup, &GainSetupWidget::readRequested);
	if(m_agcSetup)
		connect(this, &Ad9371Advanced::readRequested, m_agcSetup, &AgcSetupWidget::readRequested);
	if(m_armGpio)
		connect(this, &Ad9371Advanced::readRequested, m_armGpio, &ArmGpioWidget::readRequested);
	if(m_gpio)
		connect(this, &Ad9371Advanced::readRequested, m_gpio, &GpioWidget::readRequested);
	if(m_auxDac)
		connect(this, &Ad9371Advanced::readRequested, m_auxDac, &AuxDacWidget::readRequested);
	if(m_jesdFramer)
		connect(this, &Ad9371Advanced::readRequested, m_jesdFramer, &JesdFramerWidget::readRequested);
	if(m_jesdDeframer)
		connect(this, &Ad9371Advanced::readRequested, m_jesdDeframer, &JesdDeframerWidget::readRequested);
	if(m_bist)
		connect(this, &Ad9371Advanced::readRequested, m_bist, &BistWidget::readRequested);

	// Add all widgets to stacked widget
	m_centralWidget->addWidget(m_clkSettings);
	m_centralWidget->addWidget(m_calibrations);
	m_centralWidget->addWidget(m_txSettings);
	m_centralWidget->addWidget(m_rxSettings);
	m_centralWidget->addWidget(m_obsSettings);
	m_centralWidget->addWidget(m_gainSetup);
	m_centralWidget->addWidget(m_agcSetup);
	m_centralWidget->addWidget(m_armGpio);
	m_centralWidget->addWidget(m_gpio);
	m_centralWidget->addWidget(m_auxDac);
	m_centralWidget->addWidget(m_jesdFramer);
	m_centralWidget->addWidget(m_jesdDeframer);
	m_centralWidget->addWidget(m_bist);
}

void Ad9371Advanced::switchToSection(const QString &name)
{
	QList<QPushButton *> buttons = {m_clkSettingsBtn, m_calibrationsBtn, m_txSettingsBtn,	m_rxSettingsBtn,
					m_obsSettingsBtn, m_gainSetupBtn,    m_agcSetupBtn,	m_armGpioBtn,
					m_gpioBtn,	  m_auxDacBtn,	     m_jesdFramerBtn,	m_jesdDeframerBtn,
					m_bistBtn};

	for(QPushButton *btn : buttons) {
		if(btn && btn->text() == name) {
			btn->click();
			return;
		}
	}
	qWarning(CAT_AD9371_ADVANCED) << "Section not found:" << name;
}

QStringList Ad9371Advanced::getSections() const
{
	QStringList sections;
	QList<QPushButton *> buttons = {m_clkSettingsBtn, m_calibrationsBtn, m_txSettingsBtn,	m_rxSettingsBtn,
					m_obsSettingsBtn, m_gainSetupBtn,    m_agcSetupBtn,	m_armGpioBtn,
					m_gpioBtn,	  m_auxDacBtn,	     m_jesdFramerBtn,	m_jesdDeframerBtn,
					m_bistBtn};

	for(QPushButton *btn : buttons) {
		if(btn)
			sections.append(btn->text());
	}
	return sections;
}

void Ad9371Advanced::updateNavigationButtonsLayout()
{
	int containerWidth = m_tool->width();
	int refreshButtonWidth = m_refreshButton->width();
	int layoutSpacingAndMargins = 50;
	int availableWidth = containerWidth - layoutSpacingAndMargins - refreshButtonWidth;

	QList<QPushButton *> allButtons = {m_clkSettingsBtn, m_calibrationsBtn, m_txSettingsBtn, m_rxSettingsBtn,
					   m_obsSettingsBtn, m_gainSetupBtn,	m_agcSetupBtn,	 m_armGpioBtn,
					   m_gpioBtn,	     m_auxDacBtn,	m_jesdFramerBtn, m_jesdDeframerBtn,
					   m_bistBtn};

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

void Ad9371Advanced::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	updateNavigationButtonsLayout();
}

#include "moc_ad9371advanced.cpp"
