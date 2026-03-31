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

#include "advanced/rxsettingswidget.h"
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_RX_SETTINGS, "AD9371_RX_SETTINGS")

using namespace scopy;
using namespace scopy::ad9371;

RxSettingsWidget::RxSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_RX_SETTINGS) << "No device provided to RX Settings";
		return;
	}

	setupUi();
}

RxSettingsWidget::~RxSettingsWidget() {}

void RxSettingsWidget::setupUi()
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

	// Add RX Settings section
	contentLayout->addWidget(createRxSettingsSection(contentWidget));

	// Add RX Profile section
	contentLayout->addWidget(createRxProfileSection(contentWidget));

	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AD9371_RX_SETTINGS) << "RX Settings widget created with 12 attributes in 2 sections";
}

QWidget *RxSettingsWidget::createRxSettingsSection(QWidget *parent)
{
	MenuSectionCollapseWidget *rxSettingsSection = new MenuSectionCollapseWidget(
		"Config", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	rxSettingsSection->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// RX Channels Enable - Combo {0:RX_OFF, 1:RX1, 2:RX2, 3:RX1_RX2}
	QMap<QString, QString> rxChannelsOptions;
	rxChannelsOptions.insert("0", "RX_OFF");
	rxChannelsOptions.insert("1", "RX1");
	rxChannelsOptions.insert("2", "RX2");
	rxChannelsOptions.insert("3", "RX1_RX2");
	auto rxChannels = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,rx-settings-rx-channels-enable", rxChannelsOptions, "CHANNEL ENABLE");
	if(rxChannels) {
		if(m_widgetGroup) m_widgetGroup->add(rxChannels);
		layout->addWidget(rxChannels);
		connect(this, &RxSettingsWidget::readRequested, rxChannels, &IIOWidget::readAsync);
	}

	//  RX PLL LO Frequency (Hz) - Range Widget [300000000,6000000000,1]
	auto rxPllLoFreq =
		Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx-settings-rx-pll-lo-frequency_hz",
							    "[300000000 1 6000000000]", "RX PLL LO FREQUENCY HZ");
	if(rxPllLoFreq) {
		if(m_widgetGroup) m_widgetGroup->add(rxPllLoFreq);
		layout->addWidget(rxPllLoFreq);
		connect(this, &RxSettingsWidget::readRequested, rxPllLoFreq, &IIOWidget::readAsync);
	}

	// RX PLL Use External LO - Checkbox
	auto rxPllExtLo = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,rx-settings-rx-pll-use-external-lo", "RX PLL USE EXTERNAL LO");
	if(rxPllExtLo) {
		if(m_widgetGroup) m_widgetGroup->add(rxPllExtLo);
		layout->addWidget(rxPllExtLo);
		rxPllExtLo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &RxSettingsWidget::readRequested, rxPllExtLo, &IIOWidget::readAsync);
	}

	// Real IF Data - Checkbox
	auto realIfData = Ad9371WidgetFactory::createDebugCheckboxWidget(m_device, "adi,rx-settings-real-if-data",
									 "REAL IF DATA");
	if(realIfData) {
		if(m_widgetGroup) m_widgetGroup->add(realIfData);
		layout->addWidget(realIfData);
		realIfData->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &RxSettingsWidget::readRequested, realIfData, &IIOWidget::readAsync);
	}

	return rxSettingsSection;
}

QWidget *RxSettingsWidget::createRxProfileSection(QWidget *parent)
{
	MenuSectionCollapseWidget *rxProfileSection = new MenuSectionCollapseWidget(
		"Profile", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	rxProfileSection->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// 1. ADC Div - Range Widget [1,2,1]
	auto adcDiv =
		Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx-profile-adc-div", "[1 1 2]", "ADC DIV");
	if(adcDiv) {
		if(m_widgetGroup) m_widgetGroup->add(adcDiv);
		layout->addWidget(adcDiv);
		connect(this, &RxSettingsWidget::readRequested, adcDiv, &IIOWidget::readAsync);
	}

	// 2. RX FIR Decimation - Combo {1:"DECIMATE by 1", 2:"DECIMATE by 2", 4:"DECIMATE by 4"}
	QMap<QString, QString> firDecimationOptions;
	firDecimationOptions.insert("1", "DECIMATE by 1");
	firDecimationOptions.insert("2", "DECIMATE by 2");
	firDecimationOptions.insert("4", "DECIMATE by 4");
	auto firDecimation = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,rx-profile-rx-fir-decimation", firDecimationOptions, "RX FIR");
	if(firDecimation) {
		if(m_widgetGroup) m_widgetGroup->add(firDecimation);
		layout->addWidget(firDecimation);
		connect(this, &RxSettingsWidget::readRequested, firDecimation, &IIOWidget::readAsync);
	}

	// 3. RX DEC5 Decimation - Range Widget [4,5,1]
	auto dec5Decimation = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx-profile-rx-dec5-decimation",
									  "[4 1 5]", "RX DEC5 DECIMATION");
	if(dec5Decimation) {
		if(m_widgetGroup) m_widgetGroup->add(dec5Decimation);
		layout->addWidget(dec5Decimation);
		connect(this, &RxSettingsWidget::readRequested, dec5Decimation, &IIOWidget::readAsync);
	}

	// 4. Enable High Rejection DEC5 - Checkbox
	auto enHighRejDec5 = Ad9371WidgetFactory::createDebugCheckboxWidget(m_device, "adi,rx-profile-en-high-rej-dec5",
									    "EN HIGH REJ DEC5");
	if(enHighRejDec5) {
		if(m_widgetGroup) m_widgetGroup->add(enHighRejDec5);
		layout->addWidget(enHighRejDec5);
		enHighRejDec5->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &RxSettingsWidget::readRequested, enHighRejDec5, &IIOWidget::readAsync);
	}

	// 5. RHB1 Decimation - Range Widget [1,2,1]
	auto rhb1Decimation = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx-profile-rhb1-decimation",
									  "[1 1 2]", "RHB1 DECIMATION");
	if(rhb1Decimation) {
		if(m_widgetGroup) m_widgetGroup->add(rhb1Decimation);
		layout->addWidget(rhb1Decimation);
		connect(this, &RxSettingsWidget::readRequested, rhb1Decimation, &IIOWidget::readAsync);
	}

	// 6. IQ Rate (kHz) - Range Widget [20000,200000,1]
	auto iqRate = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx-profile-iq-rate_khz",
								  "[20000 1 200000]", "IQ RATE KHZ");
	if(iqRate) {
		if(m_widgetGroup) m_widgetGroup->add(iqRate);
		layout->addWidget(iqRate);
		connect(this, &RxSettingsWidget::readRequested, iqRate, &IIOWidget::readAsync);
	}

	// 7. RF Bandwidth (Hz) - Range Widget [5000000,100000000,1]
	auto rfBandwidth = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx-profile-rf-bandwidth_hz",
								       "[5000000 1 100000000]", "RF BANDWIDTH HZ");
	if(rfBandwidth) {
		if(m_widgetGroup) m_widgetGroup->add(rfBandwidth);
		layout->addWidget(rfBandwidth);
		connect(this, &RxSettingsWidget::readRequested, rfBandwidth, &IIOWidget::readAsync);
	}

	// 8. RX BBF 3dB Corner (kHz) - Range Widget [0,153600,1]
	auto bbf3dbCorner = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,rx-profile-rx-bbf-3db-corner_khz", "[0 1 153600]", "RX BBF 3DB CORNER KHZ");
	if(bbf3dbCorner) {
		if(m_widgetGroup) m_widgetGroup->add(bbf3dbCorner);
		layout->addWidget(bbf3dbCorner);
		connect(this, &RxSettingsWidget::readRequested, bbf3dbCorner, &IIOWidget::readAsync);
	}

	return rxProfileSection;
}
