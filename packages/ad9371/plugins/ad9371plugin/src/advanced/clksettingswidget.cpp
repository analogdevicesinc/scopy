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
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_CLK_SETTINGS, "AD9371_CLK_SETTINGS")

using namespace scopy;
using namespace scopy::ad9371;

ClkSettingsWidget::ClkSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_CLK_SETTINGS) << "No device provided to Clock Settings";
		return;
	}

	setupUi();
}

ClkSettingsWidget::~ClkSettingsWidget() {}

void ClkSettingsWidget::setupUi()
{
	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(true);

	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"Clock Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);

	createClockControls(section->contentLayout());

	contentLayout->addWidget(section);
	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AD9371_CLK_SETTINGS) << "Clock Settings widget created with 4 debug attributes";
}

void ClkSettingsWidget::createClockControls(QVBoxLayout *parentLayout)
{
	// 1. Device Clock (kHz) - DebugRangeWidget [30000,320000,1]
	IIOWidget *deviceClockWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clocks-device-clock_khz", "[30000 1 320000]", "DEVICE CLOCK KHZ");
	if(deviceClockWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(deviceClockWidget);
		Style::setStyle(deviceClockWidget, style::properties::widget::basicBackground, true, true);
		parentLayout->addWidget(deviceClockWidget);
		m_widgets.append(deviceClockWidget);
		connect(this, &ClkSettingsWidget::readRequested, deviceClockWidget, &IIOWidget::readAsync);
	}

	// 2. CLK PLL VCO Freq (kHz) - DebugRangeWidget [6000000,12500000,1]
	IIOWidget *pllVcoFreqWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clocks-clk-pll-vco-freq_khz", "[6000000 1 12500000]", "CLK PLL VCO FREQ KHZ");
	if(pllVcoFreqWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(pllVcoFreqWidget);
		Style::setStyle(pllVcoFreqWidget, style::properties::widget::basicBackground, true, true);
		parentLayout->addWidget(pllVcoFreqWidget);
		m_widgets.append(pllVcoFreqWidget);
		connect(this, &ClkSettingsWidget::readRequested, pllVcoFreqWidget, &IIOWidget::readAsync);
	}

	// 3. CLK PLL VCO Div
	QMap<QString, QString> vcoDivOptions;
	vcoDivOptions.insert("0", "VCO DIV 1");
	vcoDivOptions.insert("1", "VCO DIV 1.5");
	vcoDivOptions.insert("2", "VCO DIV 2");
	vcoDivOptions.insert("3", "VCO DIV 3");

	IIOWidget *vcoDivWidget = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,clocks-clk-pll-vco-div", vcoDivOptions, "PLL VCO DIV");
	if(vcoDivWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(vcoDivWidget);
		Style::setStyle(vcoDivWidget, style::properties::widget::basicBackground, true, true);
		parentLayout->addWidget(vcoDivWidget);
		m_widgets.append(vcoDivWidget);
		connect(this, &ClkSettingsWidget::readRequested, vcoDivWidget, &IIOWidget::readAsync);
	}

	// 4. CLK PLL HS Div - DebugRangeWidget [4,5,1]
	IIOWidget *hsDivWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,clocks-clk-pll-hs-div",
									     "[4 1 5]", "CLK PLL HS DIV");
	if(hsDivWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(hsDivWidget);
		Style::setStyle(hsDivWidget, style::properties::widget::basicBackground, true, true);
		parentLayout->addWidget(hsDivWidget);
		m_widgets.append(hsDivWidget);
		connect(this, &ClkSettingsWidget::readRequested, hsDivWidget, &IIOWidget::readAsync);
	}

	parentLayout->addStretch();
}
