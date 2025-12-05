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

#include "advanced/txsettingswidget.h"
#include "adrv9009widgetfactory.h"
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_TXSETTINGS, "TXSettings")

using namespace scopy;
using namespace scopy::adrv9009;

TxSettingsWidget::TxSettingsWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_TXSETTINGS) << "No device provided to TX Settings widget";
		return;
	}

	setupUi();
}

TxSettingsWidget::~TxSettingsWidget() {}

void TxSettingsWidget::setupUi()
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

	// Add TX Configuration section
	contentLayout->addWidget(createTxConfigurationSection(contentWidget));

	// Add TX Profile section
	contentLayout->addWidget(createTxProfileSection(contentWidget));

	// Add TX GPIO PIN Control section
	contentLayout->addWidget(createTxGpioPinControlSection(contentWidget));

	// Add spacer to push sections to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_TXSETTINGS) << "TX Settings widget created with 16 attributes in 3 collapsible sections";
}

QWidget *TxSettingsWidget::createTxProfileSection(QWidget *parent)
{
	// TX Profile section (9 attributes - 6 comboboxes + 3 range widgets)
	MenuSectionCollapseWidget *txProfileSection = new MenuSectionCollapseWidget(
		"TX Profile", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// DAC Div - Combobox [1,2] → ["1","2"]
	QMap<QString, QString> *dacDivOptions = new QMap<QString, QString>();
	dacDivOptions->insert("1", "1");
	dacDivOptions->insert("2", "2");
	auto dacDivWidget = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,tx-profile-dac-div",
									   dacDivOptions, "DAC Div");
	if(dacDivWidget) {
		txProfileSection->contentLayout()->addWidget(dacDivWidget);
		connect(this, &TxSettingsWidget::readRequested, dacDivWidget, &IIOWidget::readAsync);
	}

	// TX FIR Interpolation - Combobox [1,2,4] → ["1","2","4"]
	QMap<QString, QString> *txFirInterpOptions = new QMap<QString, QString>();
	txFirInterpOptions->insert("1", "1");
	txFirInterpOptions->insert("2", "2");
	txFirInterpOptions->insert("4", "4");
	auto txFirInterpWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,tx-profile-tx-fir-interpolation", txFirInterpOptions, "TX FIR Interpolation");
	if(txFirInterpWidget) {
		txProfileSection->contentLayout()->addWidget(txFirInterpWidget);
		connect(this, &TxSettingsWidget::readRequested, txFirInterpWidget, &IIOWidget::readAsync);
	}

	// THB1 Interpolation - Combobox [1,2] → ["1","2"]
	QMap<QString, QString> *thb1InterpOptions = new QMap<QString, QString>();
	thb1InterpOptions->insert("1", "1");
	thb1InterpOptions->insert("2", "2");
	auto thb1InterpWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,tx-profile-thb1-interpolation", thb1InterpOptions, "THB1 Interpolation");
	if(thb1InterpWidget) {
		txProfileSection->contentLayout()->addWidget(thb1InterpWidget);
		connect(this, &TxSettingsWidget::readRequested, thb1InterpWidget, &IIOWidget::readAsync);
	}

	// THB2 Interpolation - Combobox [1,2] → ["1","2"]
	QMap<QString, QString> *thb2InterpOptions = new QMap<QString, QString>();
	thb2InterpOptions->insert("1", "1");
	thb2InterpOptions->insert("2", "2");
	auto thb2InterpWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,tx-profile-thb2-interpolation", thb2InterpOptions, "THB2 Interpolation");
	if(thb2InterpWidget) {
		txProfileSection->contentLayout()->addWidget(thb2InterpWidget);
		connect(this, &TxSettingsWidget::readRequested, thb2InterpWidget, &IIOWidget::readAsync);
	}

	// THB3 Interpolation - Combobox [1,2] → ["1","2"]
	QMap<QString, QString> *thb3InterpOptions = new QMap<QString, QString>();
	thb3InterpOptions->insert("1", "1");
	thb3InterpOptions->insert("2", "2");
	auto thb3InterpWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,tx-profile-thb3-interpolation", thb3InterpOptions, "THB3 Interpolation");
	if(thb3InterpWidget) {
		txProfileSection->contentLayout()->addWidget(thb3InterpWidget);
		connect(this, &TxSettingsWidget::readRequested, thb3InterpWidget, &IIOWidget::readAsync);
	}

	// TX INT5 Interpolation - Combobox [1,5] → ["1","5"] (fixed attribute name)
	QMap<QString, QString> *txInt5InterpOptions = new QMap<QString, QString>();
	txInt5InterpOptions->insert("1", "1");
	txInt5InterpOptions->insert("5", "5");
	auto txInt5InterpWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,tx-profile-tx-int5-interpolation", txInt5InterpOptions, "TX INT5 Interpolation");
	if(txInt5InterpWidget) {
		txProfileSection->contentLayout()->addWidget(txInt5InterpWidget);
		connect(this, &TxSettingsWidget::readRequested, txInt5InterpWidget, &IIOWidget::readAsync);
	}

	// Primary Signal Bandwidth (Hz) - Range Widget (keeping as range widget per plan)
	auto primSigBwWidget =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,tx-profile-primary-sig-bandwidth_hz",
							 "[20000000 1000 200000000]", "Primary Signal Bandwidth (Hz)");
	if(primSigBwWidget) {
		txProfileSection->contentLayout()->addWidget(primSigBwWidget);
		connect(this, &TxSettingsWidget::readRequested, primSigBwWidget, &IIOWidget::readAsync);
	}

	// TX Input Rate (kHz) - Range Widget (keeping as range widget per plan)
	auto inputRateWidget = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,tx-profile-tx-input-rate_khz",
									"[30720 1 491520]", "TX Input Rate (kHz)");
	if(inputRateWidget) {
		txProfileSection->contentLayout()->addWidget(inputRateWidget);
		connect(this, &TxSettingsWidget::readRequested, inputRateWidget, &IIOWidget::readAsync);
	}

	// RF Bandwidth (Hz) - Range Widget (keeping as range widget per plan)
	auto rfBwWidget = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,tx-profile-rf-bandwidth_hz",
								   "[20000000 1000 200000000]", "RF Bandwidth (Hz)");
	if(rfBwWidget) {
		txProfileSection->contentLayout()->addWidget(rfBwWidget);
		connect(this, &TxSettingsWidget::readRequested, rfBwWidget, &IIOWidget::readAsync);
	}

	// TX DAC3D BCorner (kHz) - Range Widget (keeping as range widget per plan)
	auto dac3dBCornerWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,tx-profile-tx-dac3d-bcorner_khz", "[50000 1000 750000]", "TX DAC3D BCorner (kHz)");
	if(dac3dBCornerWidget) {
		txProfileSection->contentLayout()->addWidget(dac3dBCornerWidget);
		connect(this, &TxSettingsWidget::readRequested, dac3dBCornerWidget, &IIOWidget::readAsync);
	}

	// TX BBF 3dB Corner (kHz) - Range Widget (keeping as range widget per plan)
	auto bbf3dbWidget = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,tx-profile-tx-bbf3d-bcorner_khz",
								     "[50000 1000 750000]", "TX BBF 3dB Corner (kHz)");
	if(bbf3dbWidget) {
		txProfileSection->contentLayout()->addWidget(bbf3dbWidget);
		connect(this, &TxSettingsWidget::readRequested, bbf3dbWidget, &IIOWidget::readAsync);
	}

	return txProfileSection;
}

QWidget *TxSettingsWidget::createTxConfigurationSection(QWidget *parent)
{
	// TX Configuration section (6 attributes)
	MenuSectionCollapseWidget *txConfigSection = new MenuSectionCollapseWidget(
		"TX Configuration", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// TX Channels - Combobox with iio-osc mappings [0,1,2,3] -> [TXOFF,TX1,TX2,TX1_and_TX2]
	QMap<QString, QString> *txChannelsOptions = new QMap<QString, QString>();
	txChannelsOptions->insert("0", "TXOFF");
	txChannelsOptions->insert("1", "TX1");
	txChannelsOptions->insert("2", "TX2");
	txChannelsOptions->insert("3", "TX1_and_TX2");
	auto txChannelsWidget = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,tx-settings-tx-channels",
									       txChannelsOptions, "TX Channels Enable");
	if(txChannelsWidget) {
		txConfigSection->contentLayout()->addWidget(txChannelsWidget);
		connect(this, &TxSettingsWidget::readRequested, txChannelsWidget, &IIOWidget::readAsync);
	}

	// Deframer Selection - Combobox with iio-osc mappings [0,1,2] -> [A,B,A_and_B]
	QMap<QString, QString> *deframerOptions = new QMap<QString, QString>();
	deframerOptions->insert("0", "A");
	deframerOptions->insert("1", "B");
	deframerOptions->insert("2", "A_and_B");
	auto deframerWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,tx-settings-deframer-sel", deframerOptions, "JESD204 DEFRAMER SELECT");
	if(deframerWidget) {
		txConfigSection->contentLayout()->addWidget(deframerWidget);
		connect(this, &TxSettingsWidget::readRequested, deframerWidget, &IIOWidget::readAsync);
	}

	// TX ATTEN STEP SIZE - Combobox with iio-osc mappings [0,1,2,3] -> [0.05,0.1,0.2,0.4]
	QMap<QString, QString> *attenStepOptions = new QMap<QString, QString>();
	attenStepOptions->insert("0", "0.05");
	attenStepOptions->insert("1", "0.1");
	attenStepOptions->insert("2", "0.2");
	attenStepOptions->insert("3", "0.4");
	auto attenStepWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, "adi,tx-settings-tx-atten-step-size", attenStepOptions, "TX ATTEN STEP SIZE");
	if(attenStepWidget) {
		txConfigSection->contentLayout()->addWidget(attenStepWidget);
		connect(this, &TxSettingsWidget::readRequested, attenStepWidget, &IIOWidget::readAsync);
	}

	// TX1 Attenuation (mdB) - Range Widget (fixed attribute name with hyphen)
	auto tx1AttenWidget = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,tx-settings-tx1-atten_md-b",
								       "[0 250 41950]", "TX1 Attenuation (mdB)");
	if(tx1AttenWidget) {
		txConfigSection->contentLayout()->addWidget(tx1AttenWidget);
		connect(this, &TxSettingsWidget::readRequested, tx1AttenWidget, &IIOWidget::readAsync);
	}

	// TX2 Attenuation (mdB) - Range Widget (fixed attribute name with hyphen)
	auto tx2AttenWidget = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,tx-settings-tx2-atten_md-b",
								       "[0 250 41950]", "TX2 Attenuation (mdB)");
	if(tx2AttenWidget) {
		txConfigSection->contentLayout()->addWidget(tx2AttenWidget);
		connect(this, &TxSettingsWidget::readRequested, tx2AttenWidget, &IIOWidget::readAsync);
	}

	// Disable TX Data If PLL Unlock - Combobox [0,1,2] → [DISABLED,ZERO_DATA,RAMP_DOWN_TO_ZERO]
	QMap<QString, QString> *disableTxDataOptions = new QMap<QString, QString>();
	disableTxDataOptions->insert("0", "DISABLED");
	disableTxDataOptions->insert("1", "ZERO_DATA");
	disableTxDataOptions->insert("2", "RAMP_DOWN_TO_ZERO");
	auto disableTxDataWidget =
		Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,tx-settings-dis-tx-data-if-pll-unlock",
							       disableTxDataOptions, "Disable TX Data If PLL Unlock");
	if(disableTxDataWidget) {
		txConfigSection->contentLayout()->addWidget(disableTxDataWidget);
		connect(this, &TxSettingsWidget::readRequested, disableTxDataWidget, &IIOWidget::readAsync);
	}

	return txConfigSection;
}

QWidget *TxSettingsWidget::createTxGpioPinControlSection(QWidget *parent)
{
	// TX GPIO PIN Control section with two-column layout
	MenuSectionCollapseWidget *gpioSection = new MenuSectionCollapseWidget(
		"GPIO PIN Control", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// Create horizontal layout for two columns
	QHBoxLayout *columnsLayout = new QHBoxLayout();
	columnsLayout->setContentsMargins(0, 0, 0, 0);
	columnsLayout->setSpacing(20);

	// Create TX1 column (left)
	QWidget *tx1Column = createTxChannelGpioGroup(1, gpioSection);
	columnsLayout->addWidget(tx1Column);

	// Create TX2 column (right)
	QWidget *tx2Column = createTxChannelGpioGroup(2, gpioSection);
	columnsLayout->addWidget(tx2Column);

	// Create container widget for columns layout
	QWidget *columnsContainer = new QWidget();
	columnsContainer->setLayout(columnsLayout);

	// Add columns container to section
	gpioSection->contentLayout()->addWidget(columnsContainer);

	return gpioSection;
}

QWidget *TxSettingsWidget::createTxChannelGpioGroup(int channel, QWidget *parent)
{
	// Create container widget for TX channel GPIO controls
	QWidget *channelWidget = new QWidget(parent);
	Style::setStyle(channelWidget, style::properties::widget::border);

	// Create vertical layout for this channel
	QVBoxLayout *channelLayout = new QVBoxLayout(channelWidget);
	channelLayout->setContentsMargins(10, 10, 10, 10);
	channelLayout->setSpacing(10);

	// Add header label
	QLabel *headerLabel = new QLabel(QString("TX%1").arg(channel));
	Style::setStyle(headerLabel, style::properties::label::menuMedium);
	headerLabel->setAlignment(Qt::AlignCenter);
	channelLayout->addWidget(headerLabel);

	// Create widgets based on channel number
	QString enableAttr = QString("adi,tx%1-atten-ctrl-pin-enable").arg(channel);
	QString stepSizeAttr = QString("adi,tx%1-atten-ctrl-pin-step-size").arg(channel);
	QString incPinAttr = QString("adi,tx%1-atten-ctrl-pin-tx-atten-inc-pin").arg(channel);
	QString decPinAttr = QString("adi,tx%1-atten-ctrl-pin-tx-atten-dec-pin").arg(channel);

	// Enable checkbox
	auto enableWidget = Adrv9009WidgetFactory::createCheckboxWidget(m_device, enableAttr, "ENABLE");
	if(enableWidget) {
		channelLayout->addWidget(enableWidget);
		connect(this, &TxSettingsWidget::readRequested, enableWidget, &IIOWidget::readAsync);
	}

	// Step size range widget
	auto stepSizeWidget = Adrv9009WidgetFactory::createRangeWidget(m_device, stepSizeAttr, "[0 1 31]", "STEP SIZE");
	if(stepSizeWidget) {
		channelLayout->addWidget(stepSizeWidget);
		connect(this, &TxSettingsWidget::readRequested, stepSizeWidget, &IIOWidget::readAsync);
	}

	// Inc pin combobox - options depend on channel
	QMap<QString, QString> *incOptions = new QMap<QString, QString>();
	if(channel == 1) {
		incOptions->insert("4", "4");
		incOptions->insert("12", "12");
	} else { // channel == 2
		incOptions->insert("6", "6");
		incOptions->insert("14", "14");
	}
	auto incPinWidget =
		Adrv9009WidgetFactory::createCustomComboWidget(m_device, incPinAttr, incOptions, "TX ATTEN INC PIN");
	if(incPinWidget) {
		channelLayout->addWidget(incPinWidget);
		connect(this, &TxSettingsWidget::readRequested, incPinWidget, &IIOWidget::readAsync);
	}

	// Dec pin combobox - options depend on channel
	QMap<QString, QString> *decOptions = new QMap<QString, QString>();
	if(channel == 1) {
		decOptions->insert("5", "5");
		decOptions->insert("13", "13");
	} else { // channel == 2
		decOptions->insert("7", "7");
		decOptions->insert("15", "15");
	}
	auto decPinWidget =
		Adrv9009WidgetFactory::createCustomComboWidget(m_device, decPinAttr, decOptions, "TX ATTEN DEC PIN");
	if(decPinWidget) {
		channelLayout->addWidget(decPinWidget);
		connect(this, &TxSettingsWidget::readRequested, decPinWidget, &IIOWidget::readAsync);
	}

	return channelWidget;
}
