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
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>
#include <gui/widgets/menucollapsesection.h>
#include <gui/style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_TX_SETTINGS, "AD9371_TX_SETTINGS")

using namespace scopy;
using namespace scopy::ad9371;

TxSettingsWidget::TxSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_TX_SETTINGS) << "No device provided to TX Settings";
		return;
	}

	setupUi();
}

TxSettingsWidget::~TxSettingsWidget() {}

void TxSettingsWidget::setupUi()
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

	// Add TX Settings section
	contentLayout->addWidget(createTxSettingsSection(contentWidget));

	// Add TX Profile section
	contentLayout->addWidget(createTxProfileSection(contentWidget));

	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);
}

QWidget *TxSettingsWidget::createTxSettingsSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("Config", MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// TX Channels Enable - ComboUi {0:"TX_OFF", 1:"TX1", 2:"TX2", 3:"TX1_TX2"}
	QMap<QString, QString> txChannelsOptions;
	txChannelsOptions.insert("0", "TX OFF");
	txChannelsOptions.insert("1", "TX1");
	txChannelsOptions.insert("2", "TX2");
	txChannelsOptions.insert("3", "TX1 and TX2");
	auto txChannelsWidget = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,tx-settings-tx-channels-enable", txChannelsOptions, "CHANNEL ENABLE", this);
	if(txChannelsWidget) {
		if(m_widgetGroup) m_widgetGroup->add(txChannelsWidget);
		layout->addWidget(txChannelsWidget);
		m_widgets.append(txChannelsWidget);
		connect(this, &TxSettingsWidget::readRequested, txChannelsWidget, &IIOWidget::readAsync);
	}

	// TX PLL LO Frequency (Hz) - RangeUi [300000000,6000000000,1]
	auto txPllLoFreqWidget =
		Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,tx-settings-tx-pll-lo-frequency_hz",
							    "[300000000 1 6000000000]", "TX PLL LO FREQUENCY HZ", this);
	if(txPllLoFreqWidget) {
		if(m_widgetGroup) m_widgetGroup->add(txPllLoFreqWidget);
		layout->addWidget(txPllLoFreqWidget);
		m_widgets.append(txPllLoFreqWidget);
		connect(this, &TxSettingsWidget::readRequested, txPllLoFreqWidget, &IIOWidget::readAsync);
	}

	// TX PLL Use External LO - CheckBoxUi
	auto txPllExtLoWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,tx-settings-tx-pll-use-external-lo", "TX PLL USE EXTERNAL LO", this);
	if(txPllExtLoWidget) {
		if(m_widgetGroup) m_widgetGroup->add(txPllExtLoWidget);
		layout->addWidget(txPllExtLoWidget);
		m_widgets.append(txPllExtLoWidget);
		connect(this, &TxSettingsWidget::readRequested, txPllExtLoWidget, &IIOWidget::readAsync);
	}

	// TX Atten Step Size - ComboUi
	QMap<QString, QString> attenStepOptions;
	attenStepOptions.insert("0", "ATTEN STEP 0.05 dB");
	attenStepOptions.insert("1", "ATTEN STEP 0.10 dB");
	attenStepOptions.insert("2", "ATTEN STEP 0.20 dB");
	attenStepOptions.insert("3", "ATTEN STEP 0.40 dB");
	auto attenStepWidget = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,tx-settings-tx-atten-step-size", attenStepOptions, "STEP SIZE", this);
	if(attenStepWidget) {
		if(m_widgetGroup) m_widgetGroup->add(attenStepWidget);
		layout->addWidget(attenStepWidget);
		m_widgets.append(attenStepWidget);
		connect(this, &TxSettingsWidget::readRequested, attenStepWidget, &IIOWidget::readAsync);
	}

	// TX1 Attenuation (mdB) - RangeUi [0,41950,1]
	auto tx1AttenWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,tx-settings-tx1-atten_mdb",
									  "[0 1 41950]", "TX1 ATTEN mdB", this);
	if(tx1AttenWidget) {
		if(m_widgetGroup) m_widgetGroup->add(tx1AttenWidget);
		layout->addWidget(tx1AttenWidget);
		m_widgets.append(tx1AttenWidget);
		connect(this, &TxSettingsWidget::readRequested, tx1AttenWidget, &IIOWidget::readAsync);
	}

	// TX2 Attenuation (mdB) - RangeUi [0,41950,1]
	auto tx2AttenWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,tx-settings-tx2-atten_mdb",
									  "[0 1 41950]", "TX2 ATTEN mdB", this);
	if(tx2AttenWidget) {
		if(m_widgetGroup) m_widgetGroup->add(tx2AttenWidget);
		layout->addWidget(tx2AttenWidget);
		m_widgets.append(tx2AttenWidget);
		connect(this, &TxSettingsWidget::readRequested, tx2AttenWidget, &IIOWidget::readAsync);
	}

	return section;
}

QWidget *TxSettingsWidget::createTxProfileSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("Profile", MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// DAC Div - ComboUi
	QMap<QString, QString> dacDivOptions;
	dacDivOptions.insert("0", "DAC DIV 2");
	dacDivOptions.insert("1", "DAC DIV 2.5");
	dacDivOptions.insert("2", "DAC DIV 4");
	auto dacDivWidget = Ad9371WidgetFactory::createDebugCustomComboWidget(m_device, "adi,tx-profile-dac-div",
									      dacDivOptions, "DAC DIV", this);
	if(dacDivWidget) {
		if(m_widgetGroup) m_widgetGroup->add(dacDivWidget);
		layout->addWidget(dacDivWidget);
		m_widgets.append(dacDivWidget);
		connect(this, &TxSettingsWidget::readRequested, dacDivWidget, &IIOWidget::readAsync);
	}

	// TX FIR Interpolation - ComboUi
	QMap<QString, QString> txFirInterpOptions;
	txFirInterpOptions.insert("1", "INTERPOLATE by 1");
	txFirInterpOptions.insert("2", "INTERPOLATE by 2");
	txFirInterpOptions.insert("4", "INTERPOLATE by 4");
	auto txFirInterpWidget = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,tx-profile-tx-fir-interpolation", txFirInterpOptions, "TX FIR", this);
	if(txFirInterpWidget) {
		if(m_widgetGroup) m_widgetGroup->add(txFirInterpWidget);
		layout->addWidget(txFirInterpWidget);
		m_widgets.append(txFirInterpWidget);
		connect(this, &TxSettingsWidget::readRequested, txFirInterpWidget, &IIOWidget::readAsync);
	}

	// THB1 Interpolation - RangeUi [1,2,1]
	auto thb1InterpWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,tx-profile-thb1-interpolation", "[1 1 2]", "THB1 INTERPOLATION", this);
	if(thb1InterpWidget) {
		if(m_widgetGroup) m_widgetGroup->add(thb1InterpWidget);
		layout->addWidget(thb1InterpWidget);
		m_widgets.append(thb1InterpWidget);
		connect(this, &TxSettingsWidget::readRequested, thb1InterpWidget, &IIOWidget::readAsync);
	}

	// THB2 Interpolation - RangeUi [1,2,1]
	auto thb2InterpWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,tx-profile-thb2-interpolation", "[1 1 2]", "THB2 INTERPOLATION", this);
	if(thb2InterpWidget) {
		if(m_widgetGroup) m_widgetGroup->add(thb2InterpWidget);
		layout->addWidget(thb2InterpWidget);
		m_widgets.append(thb2InterpWidget);
		connect(this, &TxSettingsWidget::readRequested, thb2InterpWidget, &IIOWidget::readAsync);
	}

	// TX Input HB Interpolation - RangeUi [1,2,1]
	auto txInputHbInterpWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,tx-profile-tx-input-hb-interpolation", "[1 1 2]", "TX INPUT HB INTERPOLATION", this);
	if(txInputHbInterpWidget) {
		if(m_widgetGroup) m_widgetGroup->add(txInputHbInterpWidget);
		layout->addWidget(txInputHbInterpWidget);
		m_widgets.append(txInputHbInterpWidget);
		connect(this, &TxSettingsWidget::readRequested, txInputHbInterpWidget, &IIOWidget::readAsync);
	}

	// IQ Rate (kHz) - RangeUi [30000,320000,1]
	auto iqRateWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,tx-profile-iq-rate_khz",
									"[30000 1 320000]", "IQ RATE KHZ", this);
	if(iqRateWidget) {
		if(m_widgetGroup) m_widgetGroup->add(iqRateWidget);
		layout->addWidget(iqRateWidget);
		m_widgets.append(iqRateWidget);
		connect(this, &TxSettingsWidget::readRequested, iqRateWidget, &IIOWidget::readAsync);
	}

	// Primary Signal Bandwidth (Hz) - RangeUi [0,250000000,1]
	auto primSigBwWidget =
		Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,tx-profile-primary-sig-bandwidth_hz",
							    "[0 1 250000000]", "PRIMARY SIG BANDWIDTH HZ", this);
	if(primSigBwWidget) {
		if(m_widgetGroup) m_widgetGroup->add(primSigBwWidget);
		layout->addWidget(primSigBwWidget);
		m_widgets.append(primSigBwWidget);
		connect(this, &TxSettingsWidget::readRequested, primSigBwWidget, &IIOWidget::readAsync);
	}

	// RF Bandwidth (Hz) - RangeUi [0,250000000,1]
	auto rfBwWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,tx-profile-rf-bandwidth_hz",
								      "[0 1 250000000]", "RF BANDWIDTH HZ", this);
	if(rfBwWidget) {
		if(m_widgetGroup) m_widgetGroup->add(rfBwWidget);
		layout->addWidget(rfBwWidget);
		m_widgets.append(rfBwWidget);
		connect(this, &TxSettingsWidget::readRequested, rfBwWidget, &IIOWidget::readAsync);
	}

	// TX DAC 3dB Corner (kHz) - RangeUi [0,250000,1]
	auto dac3dbCornerWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,tx-profile-tx-dac-3db-corner_khz", "[0 1 250000]", "TX DAC 3DB CORNER KHZ", this);
	if(dac3dbCornerWidget) {
		if(m_widgetGroup) m_widgetGroup->add(dac3dbCornerWidget);
		layout->addWidget(dac3dbCornerWidget);
		m_widgets.append(dac3dbCornerWidget);
		connect(this, &TxSettingsWidget::readRequested, dac3dbCornerWidget, &IIOWidget::readAsync);
	}

	// TX BBF 3dB Corner (kHz) - RangeUi [0,250000,1]
	auto bbf3dbCornerWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,tx-profile-tx-bbf-3db-corner_khz", "[0 1 250000]", "TX BBF 3DB CORNER KHZ", this);
	if(bbf3dbCornerWidget) {
		if(m_widgetGroup) m_widgetGroup->add(bbf3dbCornerWidget);
		layout->addWidget(bbf3dbCornerWidget);
		m_widgets.append(bbf3dbCornerWidget);
		connect(this, &TxSettingsWidget::readRequested, bbf3dbCornerWidget, &IIOWidget::readAsync);
	}

	return section;
}
