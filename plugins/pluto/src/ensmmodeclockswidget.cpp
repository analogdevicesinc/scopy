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
 *
 */

#include "ensmmodeclockswidget.h"

#include <QLabel>
#include <iiowidgetbuilder.h>
#include <iiowidgetutils.h>
#include <style.h>
#include <iioutil/connectionprovider.h>
#include <guistrategy/comboguistrategy.h>

using namespace scopy;
using namespace pluto;

EnsmModeClocksWidget::EnsmModeClocksWidget(QString uri, QWidget *parent)
	: m_uri(uri)
	, QWidget{parent}
{
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setSpacing(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	QWidget *mainWidget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(mainWidget);
	mainWidget->setLayout(layout);
	layout->setSpacing(10);
	layout->setMargin(0);

	QScrollArea *scrollArea = new QScrollArea(parent);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(mainWidget);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	layout->addWidget(generateEnsmModeWidget(this));
	layout->addWidget(generateModeWidget(this));
	layout->addWidget(generateClocksWidget(this));

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	m_layout->addWidget(scrollArea);
}

EnsmModeClocksWidget::~EnsmModeClocksWidget()
{
	// close Connection
	ConnectionProvider::close(m_uri);
}

QWidget *EnsmModeClocksWidget::generateEnsmModeWidget(QWidget *parent)
{
	QWidget *ensmModeWidget = new QWidget(parent);
	Style::setStyle(ensmModeWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(ensmModeWidget, json::theme::background_primary);

	QVBoxLayout *ensmModeWidgetLayout = new QVBoxLayout(ensmModeWidget);
	ensmModeWidget->setLayout(ensmModeWidgetLayout);
	ensmModeWidgetLayout->setContentsMargins(5, 5, 5, 5);

	QLabel *title = new QLabel("ENSM Mode", ensmModeWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	ensmModeWidgetLayout->addWidget(title);

	// ensm_mode  ensm_mode_available
	IIOWidget *fddMode = IIOWidgetBuilder(ensmModeWidget)
				     .device(m_device)
				     .attribute("adi,frequency-division-duplex-mode-enable")
				     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				     .title("FDD/TDD")
				     .infoMessage("Use FDD mode - default TDD")
				     .buildSingle();
	ensmModeWidgetLayout->addWidget(fddMode);
	fddMode->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	fddMode->showProgressBar(false);

	// adi,ensm-enable-pin-pulse-mode-enable
	IIOWidget *ensmEnablePinPulseMode =
		IIOWidgetBuilder(ensmModeWidget)
			.device(m_device)
			.attribute("adi,ensm-enable-pin-pulse-mode-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Pin Pulse Mode")
			.infoMessage("ENSM control Pins (ENABLE/TXNRX) use Pulse mode - default Level Mode")
			.buildSingle();
	ensmModeWidgetLayout->addWidget(ensmEnablePinPulseMode);
	ensmEnablePinPulseMode->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	ensmEnablePinPulseMode->showProgressBar(false);

	// adi,ensm-enable-txnrx-control-enable
	IIOWidget *ensmEnableTxnrx =
		IIOWidgetBuilder(ensmModeWidget)
			.device(m_device)
			.attribute("adi,ensm-enable-txnrx-control-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("TXNRX Pin Control")
			.infoMessage("ENSM control Pins (ENABLE/TXNRX) control ENSM state - default SPI writes")
			.buildSingle();
	ensmModeWidgetLayout->addWidget(ensmEnableTxnrx);
	ensmEnableTxnrx->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	ensmEnableTxnrx->showProgressBar(false);

	QLabel *tddModeLabel = new QLabel("TDD Mode", ensmModeWidget);

	ensmModeWidgetLayout->addWidget(tddModeLabel);

	QHBoxLayout *ensmHBoxLayout = new QHBoxLayout();
	ensmHBoxLayout->setContentsMargins(0, 0, 0, 0);
	ensmHBoxLayout->setSpacing(10);

	// adi,tdd-use-dual-synth-mode-enable
	IIOWidget *useDualSynth =
		IIOWidgetBuilder(ensmModeWidget)
			.device(m_device)
			.attribute("adi,tdd-use-dual-synth-mode-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Use Dual Synth")
			.infoMessage("In TDD mode use Dual Synth mode - default only one Synth is enabled")
			.buildSingle();
	ensmHBoxLayout->addWidget(useDualSynth);
	useDualSynth->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	useDualSynth->showProgressBar(false);

	// the next attribute might not be available for all devices
	QString useFddVcoTableAttr = "adi,tdd-use-fdd-vco-tables-enable";

	// adi,tdd-use-fdd-vco-tables-enable
	IIOWidget *useFddVcoTable = IIOWidgetBuilder(ensmModeWidget)
					    .device(m_device)
					    .attribute(useFddVcoTableAttr)
					    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					    .title("Use FDD VCO tables")
					    .buildSingle();
	ensmHBoxLayout->addWidget(useFddVcoTable);
	useFddVcoTable->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	useFddVcoTable->showProgressBar(false);

	bool isuseFddVcoTableEnabled = true;
	if(iio_device_find_attr(m_device, useFddVcoTableAttr.toStdString().c_str()) == nullptr) {
		useFddVcoTable->setEnabled(false);
		isuseFddVcoTableEnabled = false;
		useFddVcoTable->getUiStrategy()->setInfoMessage(
			"This attribute is not available for your current device!");
	}

	// adi,tdd-skip-vco-cal-enable
	IIOWidget *skipVcoCal =
		IIOWidgetBuilder(ensmModeWidget)
			.device(m_device)
			.attribute("adi,tdd-skip-vco-cal-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Skip VCO cal")
			.infoMessage("Option to skip VCO cal in TDD mode when moving from TX/RX to Alert")
			.buildSingle();
	ensmHBoxLayout->addWidget(skipVcoCal);
	skipVcoCal->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	skipVcoCal->showProgressBar(false);

	// adi,update-tx-gain-in-alert-enable
	IIOWidget *updateTxGainInAlert =
		IIOWidgetBuilder(ensmModeWidget)
			.device(m_device)
			.attribute("adi,update-tx-gain-in-alert-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Update Tx Gain in ALERT")
			.infoMessage("in TDD mode disable immediate TX Gain update and wait until ENSM moves to Alert")
			.buildSingle();
	ensmHBoxLayout->addWidget(updateTxGainInAlert);
	updateTxGainInAlert->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	updateTxGainInAlert->showProgressBar(false);

	ensmHBoxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));
	ensmModeWidgetLayout->addLayout(ensmHBoxLayout);

	connect(this, &EnsmModeClocksWidget::readRequested, this, [=, this]() {
		fddMode->readAsync();
		ensmEnablePinPulseMode->readAsync();
		ensmEnableTxnrx->readAsync();
		useDualSynth->readAsync();
		if(isuseFddVcoTableEnabled) {
			useFddVcoTable->readAsync();
		}
		skipVcoCal->readAsync();
		updateTxGainInAlert->readAsync();
	});

	return ensmModeWidget;
}

QWidget *EnsmModeClocksWidget::generateModeWidget(QWidget *parent)
{
	QWidget *modeWidget = new QWidget(parent);
	Style::setStyle(modeWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(modeWidget, json::theme::background_primary);

	QVBoxLayout *modeWidgetLayout = new QVBoxLayout(modeWidget);
	modeWidget->setLayout(modeWidgetLayout);
	modeWidgetLayout->setContentsMargins(5, 5, 5, 5);

	QLabel *modeTitle = new QLabel("Mode", modeWidget);
	Style::setStyle(modeTitle, style::properties::label::menuBig);
	modeWidgetLayout->addWidget(modeTitle);

	// adi,rx-rf-port-input-select
	QMap<QString, QString> *rxPortInputOptions = new QMap<QString, QString>();
	rxPortInputOptions->insert("0", "(RX1A_N_&_RX1A_P)_and_(RX2A_N_&_RX2A_P)_enabled");
	rxPortInputOptions->insert("1", "(RX1B_N_&_RX1B_P)_and_(RX2B_N_&_RX2B_P)_enabled");
	rxPortInputOptions->insert("2", "(RX1C_N_&_RX1C_P)_and_(RX2C_N_&_RX2C_P)_enabled");
	rxPortInputOptions->insert("3", "RX1A_N_and_RX2A_N_enabled");
	rxPortInputOptions->insert("4", "RX1A_P_and_RX2A_P_enabled");
	rxPortInputOptions->insert("5", "RX1B_N_and_RX2B_N_enabled");
	rxPortInputOptions->insert("6", "RX1B_P_and_RX2B_P_enabled");
	rxPortInputOptions->insert("7", "RX1C_N_and_RX2C_N_enabled");
	rxPortInputOptions->insert("8", "RX1C_P_and_RX2C_P_enabled");

	auto rxValues = rxPortInputOptions->values();
	QString rxOptionasData = "";
	for(int i = 0; i < rxValues.size(); i++) {
		rxOptionasData += " " + rxValues.at(i);
	}
	IIOWidget *rxPortInput = IIOWidgetBuilder(modeWidget)
					 .device(m_device)
					 .attribute("adi,rx-rf-port-input-select")
					 .uiStrategy(IIOWidgetBuilder::ComboUi)
					 .optionsValues(rxOptionasData)
					 .title("RX port input")
					 .buildSingle();
	modeWidgetLayout->addWidget(rxPortInput);

	rxPortInput->setUItoDataConversion([this, rxPortInputOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, rxPortInputOptions);
	});
	rxPortInput->setDataToUIConversion([this, rxPortInputOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, rxPortInputOptions);
	});

	// adi,tx-rf-port-input-select
	QMap<QString, QString> *txPortInputOptions = new QMap<QString, QString>();
	txPortInputOptions->insert("0", "TX1A,TX2A");
	txPortInputOptions->insert("1", "TX1B,TX2B");

	auto txValues = txPortInputOptions->values();
	QString txOptionasData = "";
	for(int i = 0; i < txValues.size(); i++) {
		txOptionasData += " " + txValues.at(i);
	}
	IIOWidget *txPortInput = IIOWidgetBuilder(modeWidget)
					 .device(m_device)
					 .attribute("adi,tx-rf-port-input-select")
					 .uiStrategy(IIOWidgetBuilder::ComboUi)
					 .optionsValues(txOptionasData)
					 .title("TX port input")
					 .buildSingle();
	modeWidgetLayout->addWidget(txPortInput);

	txPortInput->setUItoDataConversion([this, txPortInputOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, txPortInputOptions);
	});
	txPortInput->setDataToUIConversion([this, txPortInputOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, txPortInputOptions);
	});

	// adi,rx1-rx2-phase-inversion-enable
	IIOWidget *rx1Rx2Phase = IIOWidgetBuilder(modeWidget)
					 .device(m_device)
					 .attribute("adi,rx1-rx2-phase-inversion-enable")
					 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					 .title("RX2 Phase Inversion")
					 .infoMessage("If enabled RX1 and RX2 are phase aligned")
					 .buildSingle();
	modeWidgetLayout->addWidget(rx1Rx2Phase);
	rx1Rx2Phase->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	rx1Rx2Phase->showProgressBar(false);

	connect(this, &EnsmModeClocksWidget::readRequested, this, [=, this]() {
		rxPortInput->readAsync();
		txPortInput->readAsync();
		rx1Rx2Phase->readAsync();
	});

	return modeWidget;
}

QWidget *EnsmModeClocksWidget::generateClocksWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setStyle(widget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(widget, json::theme::background_primary);

	QGridLayout *layout = new QGridLayout(widget);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);
	widget->setLayout(layout);

	QLabel *clocksTitle = new QLabel("Clocks", widget);
	Style::setStyle(clocksTitle, style::properties::label::menuBig);
	layout->addWidget(clocksTitle, 0, 0);

	// the next attribute might not be available for all devices
	QString xoDisableUseExtRefclkAttr = "adi,xo-disable-use-ext-refclk-enable";

	// adi,xo-disable-use-ext-refclk-enable
	IIOWidget *xoDisableUseExtRefclk =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute(xoDisableUseExtRefclkAttr)
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("XO Disable Use EXT RefCLK")
			.infoMessage("Disable XO use Ext CLK into XTAL_N - default XO into XTAL")
			.buildSingle();
	layout->addWidget(xoDisableUseExtRefclk, 1, 0);
	xoDisableUseExtRefclk->showProgressBar(false);

	bool isxoDisableUseExtRefclkEnabled = true;
	if(iio_device_find_attr(m_device, xoDisableUseExtRefclkAttr.toStdString().c_str()) == nullptr) {
		xoDisableUseExtRefclk->setEnabled(false);
		isxoDisableUseExtRefclkEnabled = false;
		xoDisableUseExtRefclk->getUiStrategy()->setInfoMessage(
			"This attribute is not available for your current device!");
	}

	// adi,external-rx-lo-enable
	iio_channel *altVoltage0 = iio_device_find_channel(m_device, "altvoltage0", true);
	IIOWidget *externalRxLo = IIOWidgetBuilder(widget)
					  .channel(altVoltage0)
					  .attribute("external")
					  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					  .title("Ext RX LO")
					  .infoMessage("Enables external LO for RX")
					  .buildSingle();
	layout->addWidget(externalRxLo, 2, 0);
	externalRxLo->showProgressBar(false);

	// adi,clk-output-mode-select
	QMap<QString, QString> *clkOutputModeOptions = new QMap<QString, QString>();
	clkOutputModeOptions->insert("0", "Disabled");
	clkOutputModeOptions->insert("1", "XTALN_(or_DCXO)_(buffered)");
	clkOutputModeOptions->insert("2", "ADC_CLK/2");
	clkOutputModeOptions->insert("3", "ADC_CLK/3");
	clkOutputModeOptions->insert("4", "ADC_CLK/4");
	clkOutputModeOptions->insert("5", "ADC_CLK/8");
	clkOutputModeOptions->insert("6", "ADC_CLK/16");
	clkOutputModeOptions->insert("7", "ADC_CLK/32");
	clkOutputModeOptions->insert("8", "ADC_CLK/64");

	auto values = clkOutputModeOptions->values();
	QString optionasData = "";
	for(int i = 0; i < values.size(); i++) {
		optionasData += " " + values.at(i);
	}
	IIOWidget *clkOutputMode = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,clk-output-mode-select")
					   .uiStrategy(IIOWidgetBuilder::ComboUi)
					   .optionsValues(optionasData)
					   .title("CLOCKOUT")
					   .buildSingle();
	layout->addWidget(clkOutputMode, 1, 1);

	clkOutputMode->setUItoDataConversion([this, clkOutputModeOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, clkOutputModeOptions);
	});
	clkOutputMode->setDataToUIConversion([this, clkOutputModeOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, clkOutputModeOptions);
	});

	// adi,external-tx-lo-enable
	iio_channel *altVoltage1 = iio_device_find_channel(m_device, "altvoltage1", true);
	IIOWidget *externalTxLo = IIOWidgetBuilder(widget)
					  .channel(altVoltage1)
					  .attribute("external")
					  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					  .title("Ext TX LO")
					  .infoMessage("Enables external LO for TX")
					  .buildSingle();
	layout->addWidget(externalTxLo, 3, 0);
	externalTxLo->showProgressBar(false);

	// adi,rx-fastlock-pincontrol-enable
	IIOWidget *rxFastlockPincontrol = IIOWidgetBuilder(widget)
						  .device(m_device)
						  .attribute("adi,rx-fastlock-pincontrol-enable")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("RX Fastlock Pin Control")
						  .infoMessage("RX fastlock pin control enable")
						  .buildSingle();
	layout->addWidget(rxFastlockPincontrol, 4, 0);
	rxFastlockPincontrol->showProgressBar(false);

	// adi,rx-fastlock-delay-ns
	IIOWidget *rxFastLockDelay = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,rx-fastlock-delay-ns")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .optionsValues("[0 1 63750]")
					     .title("RX Fastlock Delay (ns)")
					     .infoMessage("RX fastlock delay in ns")
					     .buildSingle();
	layout->addWidget(rxFastLockDelay, 4, 1);

	// adi,tx-fastlock-pincontrol-enable
	IIOWidget *txFastlockPincontrol = IIOWidgetBuilder(widget)
						  .device(m_device)
						  .attribute("adi,tx-fastlock-pincontrol-enable")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("TX Fastlock Pin Control")
						  .infoMessage("TX fastlock pin control enable")
						  .buildSingle();
	layout->addWidget(txFastlockPincontrol, 5, 0);
	txFastlockPincontrol->showProgressBar(false);

	// adi,tx-fastlock-delay-ns
	IIOWidget *txFastLockDelay = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,tx-fastlock-delay-ns")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .optionsValues("[0 1 63750]")
					     .title("TX Fastlock Delay (ns)")
					     .infoMessage("TX fastlock delay in ns")
					     .buildSingle();
	layout->addWidget(txFastLockDelay, 5, 1);

	connect(this, &EnsmModeClocksWidget::readRequested, this, [=, this]() {
		if(isxoDisableUseExtRefclkEnabled) {
			xoDisableUseExtRefclk->readAsync();
		}
		externalRxLo->readAsync();
		clkOutputMode->readAsync();
		externalTxLo->readAsync();
		rxFastlockPincontrol->readAsync();
		rxFastLockDelay->readAsync();
		txFastlockPincontrol->readAsync();
		txFastLockDelay->readAsync();
	});

	return widget;
}
