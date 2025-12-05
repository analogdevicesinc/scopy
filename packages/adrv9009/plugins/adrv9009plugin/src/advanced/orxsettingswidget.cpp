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

#include "advanced/orxsettingswidget.h"
#include "adrv9009widgetfactory.h"
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ORXSETTINGS, "ORXSettings")

using namespace scopy;
using namespace scopy::adrv9009;

OrxSettingsWidget::OrxSettingsWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_ORXSETTINGS) << "No device provided to ORX Settings widget";
		return;
	}

	setupUi();
}

OrxSettingsWidget::~OrxSettingsWidget() {}

void OrxSettingsWidget::setupUi()
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

	// Add ORX Configuration section
	contentLayout->addWidget(createOrxConfigSection(contentWidget));

	// Add ORX Profile section
	contentLayout->addWidget(createOrxProfileSection(contentWidget));

	// Add AUX PLL section
	contentLayout->addWidget(createAuxPllSection(contentWidget));

	// Add spacer to push sections to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_ORXSETTINGS) << "ORX Settings widget created with 12 attributes in 3 collapsible sections";
}

QWidget *OrxSettingsWidget::createOrxProfileSection(QWidget *parent)
{
	// ORX Profile section (7 attributes)
	MenuSectionCollapseWidget *orxProfileSection = new MenuSectionCollapseWidget(
		"ORX Profile", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// RX FIR Decimation - Combobox [1,2,4]
	QMap<QString, QString> *firDecimationOptions = new QMap<QString, QString>();
	firDecimationOptions->insert("1", "1");
	firDecimationOptions->insert("2", "2");
	firDecimationOptions->insert("4", "4");
	auto firDecimation = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,orx-profile-rx-fir-decimation", firDecimationOptions, "RX FIR Decimation");
	if(firDecimation) {
		orxProfileSection->contentLayout()->addWidget(firDecimation);
		connect(this, &OrxSettingsWidget::readRequested, firDecimation, &IIOWidget::readAsync);
	}

	// RX DEC5 Decimation - Combobox [4,5]
	QMap<QString, QString> *dec5DecimationOptions = new QMap<QString, QString>();
	dec5DecimationOptions->insert("4", "4");
	dec5DecimationOptions->insert("5", "5");
	auto dec5Decimation = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,orx-profile-rx-dec5-decimation", dec5DecimationOptions, "RX DEC5 Decimation");
	if(dec5Decimation) {
		orxProfileSection->contentLayout()->addWidget(dec5Decimation);
		connect(this, &OrxSettingsWidget::readRequested, dec5Decimation, &IIOWidget::readAsync);
	}

	// RHB1 Decimation - Combobox [1,2]
	QMap<QString, QString> *rhb1DecimationOptions = new QMap<QString, QString>();
	rhb1DecimationOptions->insert("1", "1");
	rhb1DecimationOptions->insert("2", "2");
	auto rhb1Decimation = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,orx-profile-rhb1-decimation", rhb1DecimationOptions, "RHB1 Decimation");
	if(rhb1Decimation) {
		orxProfileSection->contentLayout()->addWidget(rhb1Decimation);
		connect(this, &OrxSettingsWidget::readRequested, rhb1Decimation, &IIOWidget::readAsync);
	}

	// ORX Output Rate (kHz) - Range Widget
	auto outputRate = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,orx-profile-orx-output-rate_khz",
								   "[30625 1 500000]", "ORX Output Rate (kHz)");
	if(outputRate) {
		orxProfileSection->contentLayout()->addWidget(outputRate);
		connect(this, &OrxSettingsWidget::readRequested, outputRate, &IIOWidget::readAsync);
	}

	// RF Bandwidth (Hz) - Range Widget
	auto rfBandwidth = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,orx-profile-rf-bandwidth_hz",
								    "[5000000 1 450000000]", "RF Bandwidth (Hz)");
	if(rfBandwidth) {
		orxProfileSection->contentLayout()->addWidget(rfBandwidth);
		connect(this, &OrxSettingsWidget::readRequested, rfBandwidth, &IIOWidget::readAsync);
	}

	// RX BBF3D BCorner (kHz) - Range Widget
	auto bbf3dCorner = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,orx-profile-rx-bbf3d-bcorner_khz",
								    "[10000 1 400000]", "RX BBF3D BCorner (kHz)");
	if(bbf3dCorner) {
		orxProfileSection->contentLayout()->addWidget(bbf3dCorner);
		connect(this, &OrxSettingsWidget::readRequested, bbf3dCorner, &IIOWidget::readAsync);
	}

	// ORX DDC Mode - Combobox with iio-osc mapping [7] -> [0]
	QMap<QString, QString> *ddcModeOptions = new QMap<QString, QString>();
	ddcModeOptions->insert("7", "0");
	auto ddcMode = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,orx-profile-orx-ddc-mode",
								      ddcModeOptions, "ORX DDC Mode");
	if(ddcMode) {
		orxProfileSection->contentLayout()->addWidget(ddcMode);
		connect(this, &OrxSettingsWidget::readRequested, ddcMode, &IIOWidget::readAsync);
	}

	return orxProfileSection;
}

QWidget *OrxSettingsWidget::createOrxConfigSection(QWidget *parent)
{
	// ORX Configuration section (3 attributes)
	MenuSectionCollapseWidget *orxConfigSection = new MenuSectionCollapseWidget(
		"ORX Configuration", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// ORX Channels Enable - Combobox [0,1,2,3] → [OFF,ORX1,ORX2,ORX1_and_ORX2]
	QMap<QString, QString> *orxChannelsOptions = new QMap<QString, QString>();
	orxChannelsOptions->insert("0", "OFF");
	orxChannelsOptions->insert("1", "ORX1");
	orxChannelsOptions->insert("2", "ORX2");
	orxChannelsOptions->insert("3", "ORX1_and_ORX2");
	auto orxChannels = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,obs-settings-obs-rx-channels-enable", orxChannelsOptions, "ORX Channels Enable");
	if(orxChannels) {
		orxConfigSection->contentLayout()->addWidget(orxChannels);
		connect(this, &OrxSettingsWidget::readRequested, orxChannels, &IIOWidget::readAsync);
	}

	// JESD204 Framer Selection - Combobox [0,1,2] → [A,B,A_and_B]
	QMap<QString, QString> *framerSelOptions = new QMap<QString, QString>();
	framerSelOptions->insert("0", "A");
	framerSelOptions->insert("1", "B");
	framerSelOptions->insert("2", "A_and_B");
	auto framerSel = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,obs-settings-framer-sel",
									framerSelOptions, "JESD204 Framer Selection");
	if(framerSel) {
		orxConfigSection->contentLayout()->addWidget(framerSel);
		connect(this, &OrxSettingsWidget::readRequested, framerSel, &IIOWidget::readAsync);
	}

	// ORX LO Source - Combobox [0,1] → [RFPLL,AUXPLL]
	QMap<QString, QString> *loSourceOptions = new QMap<QString, QString>();
	loSourceOptions->insert("0", "RFPLL");
	loSourceOptions->insert("1", "AUXPLL");
	auto loSource = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,obs-settings-obs-rx-lo-source",
								       loSourceOptions, "ORX LO Source");
	if(loSource) {
		orxConfigSection->contentLayout()->addWidget(loSource);
		connect(this, &OrxSettingsWidget::readRequested, loSource, &IIOWidget::readAsync);
	}

	return orxConfigSection;
}

QWidget *OrxSettingsWidget::createAuxPllSection(QWidget *parent)
{
	// AUX PLL section (2 attributes)
	MenuSectionCollapseWidget *auxPllSection = new MenuSectionCollapseWidget(
		"AUX PLL", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// GPIO Select - Range Widget
	auto gpioSelect = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,orx-lo-cfg-gpio-select", "[0 1 19]",
								   "GPIO Select");
	if(gpioSelect) {
		auxPllSection->contentLayout()->addWidget(gpioSelect);
		connect(this, &OrxSettingsWidget::readRequested, gpioSelect, &IIOWidget::readAsync);
	}

	// Disable AUX PLL Relocking - Checkbox
	auto disableAuxPllRelock = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,orx-lo-cfg-disable-aux-pll-relocking", "Disable AUX PLL Relocking");
	if(disableAuxPllRelock) {
		auxPllSection->contentLayout()->addWidget(disableAuxPllRelock);
		connect(this, &OrxSettingsWidget::readRequested, disableAuxPllRelock, &IIOWidget::readAsync);
	}

	return auxPllSection;
}
