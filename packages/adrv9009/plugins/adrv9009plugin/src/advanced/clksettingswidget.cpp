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
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_CLKSETTINGS, "CLKSettings")

using namespace scopy;
using namespace scopy::adrv9009;

ClkSettingsWidget::ClkSettingsWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_CLKSETTINGS) << "No device provided to CLK Settings widget";
		return;
	}

	setupUi();
}

ClkSettingsWidget::~ClkSettingsWidget() {}

void ClkSettingsWidget::setupUi()
{
	// Main layout for this widget
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	// Create scroll area for all sections
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// Create content widget for scroll area
	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	// Add CLK Settings section
	contentLayout->addWidget(createClkSettingsSection(contentWidget));

	// Add spacer to push sections to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_CLKSETTINGS) << "CLK Settings widget created with 5 attributes in 1 collapsible section";
}

QWidget *ClkSettingsWidget::createClkSettingsSection(QWidget *parent)
{
	// CLK Settings section (5 attributes)
	MenuSectionCollapseWidget *clkSection = new MenuSectionCollapseWidget(
		"CLK Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	clkSection->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// Device Clock (kHz) - Range Widget [10000 1 1000000]
	auto deviceClockWidget = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,dig-clocks-device-clock_khz",
									  "[10000 1 1000000]", "Device Clock (kHz)");
	if(deviceClockWidget) {
		layout->addWidget(deviceClockWidget);
		connect(this, &ClkSettingsWidget::readRequested, deviceClockWidget, &IIOWidget::readAsync);
	}

	// CLK PLL VCO Frequency (kHz) - Range Widget [6000000 1 12000000]
	auto vcoFreqWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,dig-clocks-clk-pll-vco-freq_khz", "[6000000 1 12000000]", "CLK PLL VCO Frequency (kHz)");
	if(vcoFreqWidget) {
		layout->addWidget(vcoFreqWidget);
		connect(this, &ClkSettingsWidget::readRequested, vcoFreqWidget, &IIOWidget::readAsync);
	}

	// CLK PLL HS DIV - Combobox with iio-osc mappings [0,1,2,3,4] -> [2,2.5,3,4,5]
	QMap<QString, QString> *hsDivOptions = new QMap<QString, QString>();
	hsDivOptions->insert("0", "2");
	hsDivOptions->insert("1", "2.5");
	hsDivOptions->insert("2", "3");
	hsDivOptions->insert("3", "4");
	hsDivOptions->insert("4", "5");
	auto hsDivWidget = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,dig-clocks-clk-pll-hs-div",
									  hsDivOptions, "CLK PLL HS DIV");
	if(hsDivWidget) {
		layout->addWidget(hsDivWidget);
		connect(this, &ClkSettingsWidget::readRequested, hsDivWidget, &IIOWidget::readAsync);
	}

	// RF PLL Phase Sync Mode - Combobox with iio-osc mappings [0,1,2,3] ->
	// [NOSYNC,INIT_TRACK,INIT_1TRACK,INIT_CONTTRACK]
	QMap<QString, QString> *phaseSyncModeOptions = new QMap<QString, QString>();
	phaseSyncModeOptions->insert("0", "NOSYNC");
	phaseSyncModeOptions->insert("1", "INIT_TRACK");
	phaseSyncModeOptions->insert("2", "INIT_1TRACK");
	phaseSyncModeOptions->insert("3", "INIT_CONTTRACK");
	auto phaseSyncModeWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,dig-clocks-rf-pll-phase-sync-mode", phaseSyncModeOptions, "RF PLL Phase Sync Mode");
	if(phaseSyncModeWidget) {
		layout->addWidget(phaseSyncModeWidget);
		connect(this, &ClkSettingsWidget::readRequested, phaseSyncModeWidget, &IIOWidget::readAsync);
	}

	// RF PLL Use External LO - Checkbox
	auto useExternalLoWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,dig-clocks-rf-pll-use-external-lo", "RF PLL Use External LO");
	if(useExternalLoWidget) {
		layout->addWidget(useExternalLoWidget);
		useExternalLoWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ClkSettingsWidget::readRequested, useExternalLoWidget, &IIOWidget::readAsync);
	}

	return clkSection;
}
