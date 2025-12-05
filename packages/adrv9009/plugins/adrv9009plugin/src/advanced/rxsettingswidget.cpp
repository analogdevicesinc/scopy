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
#include "adrv9009widgetfactory.h"
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_RXSETTINGS, "RXSettings")

using namespace scopy;
using namespace scopy::adrv9009;

RxSettingsWidget::RxSettingsWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_RXSETTINGS) << "No device provided to RX Settings widget";
		return;
	}

	setupUi();
}

RxSettingsWidget::~RxSettingsWidget() {}

void RxSettingsWidget::setupUi()
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

	// Add RX Configuration section
	contentLayout->addWidget(createRxConfigurationSection(contentWidget));

	// Add RX Profile section
	contentLayout->addWidget(createRxProfileSection(contentWidget));

	// Add NCO Shifter Config section
	contentLayout->addWidget(createNcoShifterConfigSection(contentWidget));

	// Add RX GPIO PIN Control section
	contentLayout->addWidget(createRxGpioPinControlSection(contentWidget));

	// Add spacer to push sections to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_RXSETTINGS) << "RX Settings widget created with 25 attributes in 4 collapsible sections";
}

QWidget *RxSettingsWidget::createRxProfileSection(QWidget *parent)
{
	// RX Profile section (7 attributes)
	MenuSectionCollapseWidget *rxProfileSection = new MenuSectionCollapseWidget(
		"RX Profile", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// RX FIR Decimation - Combobox [1,2,4]
	QMap<QString, QString> *firDecimationOptions = new QMap<QString, QString>();
	firDecimationOptions->insert("1", "1");
	firDecimationOptions->insert("2", "2");
	firDecimationOptions->insert("4", "4");
	auto firDecimation = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,rx-profile-rx-fir-decimation", firDecimationOptions, "RX FIR Decimation");
	if(firDecimation) {
		rxProfileSection->contentLayout()->addWidget(firDecimation);
		connect(this, &RxSettingsWidget::readRequested, firDecimation, &IIOWidget::readAsync);
	}

	// RX DEC5 Decimation - Combobox [4,5]
	QMap<QString, QString> *dec5DecimationOptions = new QMap<QString, QString>();
	dec5DecimationOptions->insert("4", "4");
	dec5DecimationOptions->insert("5", "5");
	auto dec5Decimation = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,rx-profile-rx-dec5-decimation", dec5DecimationOptions, "RX DEC5 Decimation");
	if(dec5Decimation) {
		rxProfileSection->contentLayout()->addWidget(dec5Decimation);
		connect(this, &RxSettingsWidget::readRequested, dec5Decimation, &IIOWidget::readAsync);
	}

	// RHB1 Decimation - Combobox [1,2]
	QMap<QString, QString> *rhb1DecimationOptions = new QMap<QString, QString>();
	rhb1DecimationOptions->insert("1", "1");
	rhb1DecimationOptions->insert("2", "2");
	auto rhb1Decimation = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,rx-profile-rhb1-decimation",
									     rhb1DecimationOptions, "RHB1 Decimation");
	if(rhb1Decimation) {
		rxProfileSection->contentLayout()->addWidget(rhb1Decimation);
		connect(this, &RxSettingsWidget::readRequested, rhb1Decimation, &IIOWidget::readAsync);
	}

	// RX Output Rate (kHz) - Range Widget
	auto outputRate = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-profile-rx-output-rate_khz",
								   "[25000 1 370000]", "RX Output Rate (kHz)");
	if(outputRate) {
		rxProfileSection->contentLayout()->addWidget(outputRate);
		connect(this, &RxSettingsWidget::readRequested, outputRate, &IIOWidget::readAsync);
	}

	// RF Bandwidth (Hz) - Range Widget
	auto rfBandwidth = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-profile-rf-bandwidth_hz",
								    "[5000000 1 200000000]", "RF Bandwidth (Hz)");
	if(rfBandwidth) {
		rxProfileSection->contentLayout()->addWidget(rfBandwidth);
		connect(this, &RxSettingsWidget::readRequested, rfBandwidth, &IIOWidget::readAsync);
	}

	// RX BBF3D BCorner (kHz) - Range Widget
	auto bbf3dCorner = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-profile-rx-bbf3d-bcorner_khz",
								    "[5000 1 200000]", "RX BBF3D BCorner (kHz)");
	if(bbf3dCorner) {
		rxProfileSection->contentLayout()->addWidget(bbf3dCorner);
		connect(this, &RxSettingsWidget::readRequested, bbf3dCorner, &IIOWidget::readAsync);
	}

	// RX DDC Mode - Combobox with complete iio-osc mappings [0,1,2,3,4,5,6,7]
	QMap<QString, QString> *ddcModeOptions = new QMap<QString, QString>();
	ddcModeOptions->insert("0", "BYPASS");
	ddcModeOptions->insert("1", "FILTERONLY");
	ddcModeOptions->insert("2", "INT2");
	ddcModeOptions->insert("3", "DEC2");
	ddcModeOptions->insert("4", "BYPASS_REALIF");
	ddcModeOptions->insert("5", "FILTERONLY_REALIF");
	ddcModeOptions->insert("6", "INT2_REALIF");
	ddcModeOptions->insert("7", "DEC2_REALIF");
	auto ddcMode = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,rx-profile-rx-ddc-mode",
								      ddcModeOptions, "RX DDC Mode");
	if(ddcMode) {
		rxProfileSection->contentLayout()->addWidget(ddcMode);
		connect(this, &RxSettingsWidget::readRequested, ddcMode, &IIOWidget::readAsync);
	}

	return rxProfileSection;
}

QWidget *RxSettingsWidget::createNcoShifterConfigSection(QWidget *parent)
{
	// NCO Shifter Config section (8 attributes)
	MenuSectionCollapseWidget *ncoSection = new MenuSectionCollapseWidget(
		"NCO Shifter Config", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// Band A - Input Band Width (kHz) - Range Widget
	auto bandAInputBW =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-nco-shifter-band-a-input-band-width_khz",
							 "[0 1 4294967295]", "Band A Input Band Width (kHz)");
	if(bandAInputBW) {
		ncoSection->contentLayout()->addWidget(bandAInputBW);
		connect(this, &RxSettingsWidget::readRequested, bandAInputBW, &IIOWidget::readAsync);
	}

	// Band A - Input Center Freq (kHz) - Range Widget
	auto bandAInputCenter = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rx-nco-shifter-band-a-input-center-freq_khz", "[-2147483647 1 2147483647]",
		"Band A Input Center Freq (kHz)");
	if(bandAInputCenter) {
		ncoSection->contentLayout()->addWidget(bandAInputCenter);
		connect(this, &RxSettingsWidget::readRequested, bandAInputCenter, &IIOWidget::readAsync);
	}

	// Band A - NCO1 Freq (kHz) - Range Widget
	auto bandANCO1 =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-nco-shifter-band-a-nco1-freq_khz",
							 "[-2147483647 1 2147483647]", "Band A NCO1 Freq (kHz)");
	if(bandANCO1) {
		ncoSection->contentLayout()->addWidget(bandANCO1);
		connect(this, &RxSettingsWidget::readRequested, bandANCO1, &IIOWidget::readAsync);
	}

	// Band A - NCO2 Freq (kHz) - Range Widget
	auto bandANCO2 =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-nco-shifter-band-a-nco2-freq_khz",
							 "[-2147483647 1 2147483647]", "Band A NCO2 Freq (kHz)");
	if(bandANCO2) {
		ncoSection->contentLayout()->addWidget(bandANCO2);
		connect(this, &RxSettingsWidget::readRequested, bandANCO2, &IIOWidget::readAsync);
	}

	// Band B - Input Band Width (kHz) - Range Widget
	auto bandBInputBW =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-nco-shifter-band-binput-band-width_khz",
							 "[0 1 4294967295]", "Band B Input Band Width (kHz)");
	if(bandBInputBW) {
		ncoSection->contentLayout()->addWidget(bandBInputBW);
		connect(this, &RxSettingsWidget::readRequested, bandBInputBW, &IIOWidget::readAsync);
	}

	// Band B - Input Center Freq (kHz) - Range Widget
	auto bandBInputCenter = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rx-nco-shifter-band-binput-center-freq_khz", "[-2147483647 1 2147483647]",
		"Band B Input Center Freq (kHz)");
	if(bandBInputCenter) {
		ncoSection->contentLayout()->addWidget(bandBInputCenter);
		connect(this, &RxSettingsWidget::readRequested, bandBInputCenter, &IIOWidget::readAsync);
	}

	// Band B - NCO1 Freq (kHz) - Range Widget
	auto bandBNCO1 =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-nco-shifter-band-bnco1-freq_khz",
							 "[-2147483647 1 2147483647]", "Band B NCO1 Freq (kHz)");
	if(bandBNCO1) {
		ncoSection->contentLayout()->addWidget(bandBNCO1);
		connect(this, &RxSettingsWidget::readRequested, bandBNCO1, &IIOWidget::readAsync);
	}

	// Band B - NCO2 Freq (kHz) - Range Widget
	auto bandBNCO2 =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-nco-shifter-band-bnco2-freq_khz",
							 "[-2147483647 1 2147483647]", "Band B NCO2 Freq (kHz)");
	if(bandBNCO2) {
		ncoSection->contentLayout()->addWidget(bandBNCO2);
		connect(this, &RxSettingsWidget::readRequested, bandBNCO2, &IIOWidget::readAsync);
	}

	return ncoSection;
}

QWidget *RxSettingsWidget::createRxGpioPinControlSection(QWidget *parent)
{
	// RX GPIO PIN Control section with two-column layout
	MenuSectionCollapseWidget *gpioSection = new MenuSectionCollapseWidget(
		"GPIO PIN Control", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// Create horizontal layout for two columns
	QHBoxLayout *columnsLayout = new QHBoxLayout();
	columnsLayout->setContentsMargins(0, 0, 0, 0);
	columnsLayout->setSpacing(20);

	// Create RX1 column (left)
	QWidget *rx1Column = createRxChannelGpioGroup(1, gpioSection);
	columnsLayout->addWidget(rx1Column);

	// Create RX2 column (right)
	QWidget *rx2Column = createRxChannelGpioGroup(2, gpioSection);
	columnsLayout->addWidget(rx2Column);

	// Create container widget for columns layout
	QWidget *columnsContainer = new QWidget();
	columnsContainer->setLayout(columnsLayout);

	// Add columns container to section
	gpioSection->contentLayout()->addWidget(columnsContainer);

	return gpioSection;
}

QWidget *RxSettingsWidget::createRxChannelGpioGroup(int channel, QWidget *parent)
{
	// Create container widget for RX channel GPIO controls
	QWidget *channelWidget = new QWidget(parent);
	Style::setStyle(channelWidget, style::properties::widget::border);

	// Create vertical layout for this channel
	QVBoxLayout *channelLayout = new QVBoxLayout(channelWidget);
	channelLayout->setContentsMargins(10, 10, 10, 10);
	channelLayout->setSpacing(10);

	// Add header label
	QLabel *headerLabel = new QLabel(QString("RX%1").arg(channel));
	Style::setStyle(headerLabel, style::properties::label::menuMedium);
	headerLabel->setAlignment(Qt::AlignCenter);
	channelLayout->addWidget(headerLabel);

	// Create widgets based on channel number
	QString enableAttr = QString("adi,rx%1-gain-ctrl-pin-enable").arg(channel);
	QString incStepAttr = QString("adi,rx%1-gain-ctrl-pin-inc-step").arg(channel);
	QString decStepAttr = QString("adi,rx%1-gain-ctrl-pin-dec-step").arg(channel);
	QString incPinAttr = QString("adi,rx%1-gain-ctrl-pin-rx-gain-inc-pin").arg(channel);
	QString decPinAttr = QString("adi,rx%1-gain-ctrl-pin-rx-gain-dec-pin").arg(channel);

	// Enable checkbox
	auto enableWidget = Adrv9009WidgetFactory::createCheckboxWidget(m_device, enableAttr, "ENABLE");
	if(enableWidget) {
		channelLayout->addWidget(enableWidget);
		connect(this, &RxSettingsWidget::readRequested, enableWidget, &IIOWidget::readAsync);
	}

	// Inc step range widget
	auto incStepWidget = Adrv9009WidgetFactory::createRangeWidget(m_device, incStepAttr, "[0 1 7]", "INC STEP");
	if(incStepWidget) {
		channelLayout->addWidget(incStepWidget);
		connect(this, &RxSettingsWidget::readRequested, incStepWidget, &IIOWidget::readAsync);
	}

	// Dec step range widget
	auto decStepWidget = Adrv9009WidgetFactory::createRangeWidget(m_device, decStepAttr, "[0 1 7]", "DEC STEP");
	if(decStepWidget) {
		channelLayout->addWidget(decStepWidget);
		connect(this, &RxSettingsWidget::readRequested, decStepWidget, &IIOWidget::readAsync);
	}

	// Gain inc pin combobox - options depend on channel
	QMap<QString, QString> *incOptions = new QMap<QString, QString>();
	if(channel == 1) {
		incOptions->insert("0", "0");
		incOptions->insert("10", "10");
	} else { // channel == 2
		incOptions->insert("3", "3");
		incOptions->insert("13", "13");
	}
	auto incPinWidget =
		Adrv9009WidgetFactory::createCustomComboWidget(m_device, incPinAttr, incOptions, "RX GAIN INC PIN");
	if(incPinWidget) {
		channelLayout->addWidget(incPinWidget);
		connect(this, &RxSettingsWidget::readRequested, incPinWidget, &IIOWidget::readAsync);
	}

	// Gain dec pin combobox - options depend on channel
	QMap<QString, QString> *decOptions = new QMap<QString, QString>();
	if(channel == 1) {
		decOptions->insert("1", "1");
		decOptions->insert("11", "11");
	} else { // channel == 2
		decOptions->insert("4", "4");
		decOptions->insert("14", "14");
	}
	auto decPinWidget =
		Adrv9009WidgetFactory::createCustomComboWidget(m_device, decPinAttr, decOptions, "RX GAIN DEC PIN");
	if(decPinWidget) {
		channelLayout->addWidget(decPinWidget);
		connect(this, &RxSettingsWidget::readRequested, decPinWidget, &IIOWidget::readAsync);
	}

	return channelWidget;
}

QWidget *RxSettingsWidget::createRxConfigurationSection(QWidget *parent)
{
	// RX Configuration section (2 attributes)
	MenuSectionCollapseWidget *rxConfigSection = new MenuSectionCollapseWidget(
		"RX Configuration", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// RX Channels - Combobox with iio-osc mappings [0,1,2,3] -> [RXOFF,RX1,RX2,RX1_and_RX2]
	QMap<QString, QString> *rxChannelsOptions = new QMap<QString, QString>();
	rxChannelsOptions->insert("0", "RXOFF");
	rxChannelsOptions->insert("1", "RX1");
	rxChannelsOptions->insert("2", "RX2");
	rxChannelsOptions->insert("3", "RX1_and_RX2");
	auto rxChannels = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,rx-settings-rx-channels",
									 rxChannelsOptions, "RX Channel Enable");
	if(rxChannels) {
		rxConfigSection->contentLayout()->addWidget(rxChannels);
		connect(this, &RxSettingsWidget::readRequested, rxChannels, &IIOWidget::readAsync);
	}

	// Framer Selection - Combobox [0,1,2]
	QMap<QString, QString> *framerSelOptions = new QMap<QString, QString>();
	framerSelOptions->insert("0", "A");
	framerSelOptions->insert("1", "B");
	framerSelOptions->insert("2", "A_and_B");
	auto framerSel = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,rx-settings-framer-sel",
									framerSelOptions, "JESD204 Framer Selection");
	if(framerSel) {
		rxConfigSection->contentLayout()->addWidget(framerSel);
		connect(this, &RxSettingsWidget::readRequested, framerSel, &IIOWidget::readAsync);
	}

	return rxConfigSection;
}
