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

#include "advanced/clksettingswidget.h"
#include "adrv9009widgetfactory.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_ADRV9009_CLK_SETTINGS, "ADRV9009_CLK_SETTINGS")

using namespace scopy;
using namespace scopy::adrv9009;

ClkSettingsWidget::ClkSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_ADRV9009_CLK_SETTINGS) << "No device provided to Clock Settings";
		return;
	}

	const char *device_name = iio_device_get_name(m_device);
	qDebug(CAT_ADRV9009_CLK_SETTINGS)
		<< "Clock Settings initialized for device:" << (device_name ? device_name : "unknown");

	setupUi();
}

ClkSettingsWidget::~ClkSettingsWidget() {}

void ClkSettingsWidget::setupUi()
{
	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	// Create scroll area for the content
	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(true);

	// Content widget with proper Scopy styling
	// Create content widget for scroll area
	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	// Create collapsible Clock Settings section
	MenuSectionCollapseWidget *clockSection = new MenuSectionCollapseWidget(
		"Clock Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);

	createClockControls(clockSection->contentLayout());
	contentLayout->addWidget(clockSection);

	// Add spacer to push content to top
	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);
}

void ClkSettingsWidget::createClockControls(QVBoxLayout *parentLayout)
{
	// Create widgets in the order shown in iio-oscilloscope screenshot:
	// 1. Device Clock (kHz)
	// 2. CLK PLL VCO Freq (kHz)
	// 3. CLK PLL HS Div
	// 4. RF PLL Phase Sync Mode
	// 5. RF PLL Use External LO

	// 1. Device Clock (kHz) - RangeWidget
	IIOWidget *deviceClockWidget =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,dig-clocks-device-clock_khz",
							 "[10000 1000 500000]", "Device Clock (kHz)", m_widgetGroup);
	if(deviceClockWidget) {
		Style::setStyle(deviceClockWidget, style::properties::widget::basicBackground, true, true);
		parentLayout->addWidget(deviceClockWidget);
	}

	// 2. CLK PLL VCO Freq (kHz) - RangeWidget
	IIOWidget *pllVcoFreqWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,dig-clocks-clk-pll-vco-freq_khz", "[5000000 1000 12000000]", "CLK PLL VCO Freq (kHz)",
		m_widgetGroup);
	if(pllVcoFreqWidget) {
		Style::setStyle(pllVcoFreqWidget, style::properties::widget::basicBackground, true, true);
		parentLayout->addWidget(pllVcoFreqWidget);
	}

	// 3. CLK PLL HS Div - ComboWidget
	QMap<QString, QString> hsDivOptions;
	hsDivOptions.insert("0", "2");
	hsDivOptions.insert("1", "2.5");
	hsDivOptions.insert("2", "3");
	hsDivOptions.insert("3", "4");
	hsDivOptions.insert("4", "5");

	IIOWidget *hsDividerWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,dig-clocks-clk-pll-hs-div", hsDivOptions, "CLK PLL HS Divider", m_widgetGroup);
	if(hsDividerWidget) {
		Style::setStyle(hsDividerWidget, style::properties::widget::basicBackground, true, true);
		parentLayout->addWidget(hsDividerWidget);
	}

	// 4. RF PLL Phase Sync Mode - ComboWidget
	QMap<QString, QString> phaseSyncOptions;
	phaseSyncOptions.insert("0", "NOSYNC");
	phaseSyncOptions.insert("1", "INIT_TRACK");
	phaseSyncOptions.insert("2", "INIT_1TRACK");
	phaseSyncOptions.insert("3", "INIT_CONTTRACK");

	IIOWidget *phaseSyncModeWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,dig-clocks-rf-pll-phase-sync-mode", phaseSyncOptions, "Phase Sync Mode");
	if(phaseSyncModeWidget) {
		Style::setStyle(phaseSyncModeWidget, style::properties::widget::basicBackground, true, true);
		parentLayout->addWidget(phaseSyncModeWidget);
	}

	// 5. RF PLL Use External LO - CheckboxWidget
	IIOWidget *externalLoWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,dig-clocks-rf-pll-use-external-lo", "Use External LO");
	if(externalLoWidget) {
		Style::setStyle(externalLoWidget, style::properties::widget::basicBackground, true, true);
		parentLayout->addWidget(externalLoWidget);
	}

	// Add stretch to push content to top
	parentLayout->addStretch();

	// Connect refresh signals
	connect(this, &ClkSettingsWidget::readRequested, deviceClockWidget, &IIOWidget::readAsync);
	connect(this, &ClkSettingsWidget::readRequested, pllVcoFreqWidget, &IIOWidget::readAsync);
	connect(this, &ClkSettingsWidget::readRequested, hsDividerWidget, &IIOWidget::readAsync);
	connect(this, &ClkSettingsWidget::readRequested, phaseSyncModeWidget, &IIOWidget::readAsync);
	connect(this, &ClkSettingsWidget::readRequested, externalLoWidget, &IIOWidget::readAsync);
}
