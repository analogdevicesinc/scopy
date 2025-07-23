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

#include "txmonitorwidget.h"

#include <style.h>
#include <iiowidgetbuilder.h>
#include <iiowidgetutils.h>

using namespace scopy;
using namespace ad936x;

TxMonitorWidget::TxMonitorWidget(iio_device *device, QWidget *parent)
	: m_device(device)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);

	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	m_layout->addWidget(widget);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QGridLayout *gLayout1 = new QGridLayout();

	QLabel *txMonitorTitle = new QLabel("Tx Monitor", widget);
	Style::setStyle(txMonitorTitle, style::properties::label::menuBig);
	layout->addWidget(txMonitorTitle);

	gLayout1->addWidget(new QLabel("TX1", widget), 0, 1);
	gLayout1->addWidget(new QLabel("TX2", widget), 0, 2);

	gLayout1->addWidget(new QLabel("Frontend Gain", widget), 1, 0);
	gLayout1->addWidget(new QLabel("LO Common Mode", widget), 2, 0);

	QMap<QString, QString> *txFrontendGainOptions = new QMap<QString, QString>();
	txFrontendGainOptions->insert("0", "Open");
	txFrontendGainOptions->insert("1", "0dB");
	txFrontendGainOptions->insert("2", "6dB");
	txFrontendGainOptions->insert("3", "9.5dB");

	auto values = txFrontendGainOptions->values();
	QString optionasData = "";
	for(int i = 0; i < values.size(); i++) {
		optionasData += " " + values.at(i);
	}

	// adi,txmon-1-front-end-gain
	IIOWidget *tx1FrontendGain = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,txmon-1-front-end-gain")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .optionsValues(optionasData)
					     .title("")
					     .buildSingle();
	gLayout1->addWidget(tx1FrontendGain, 1, 1);

	tx1FrontendGain->setUItoDataConversion([this, txFrontendGainOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, txFrontendGainOptions);
	});
	tx1FrontendGain->setDataToUIConversion([this, txFrontendGainOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, txFrontendGainOptions);
	});

	// adi,txmon-2-front-end-gain
	IIOWidget *tx2FrontendGain = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,txmon-2-front-end-gain")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .optionsValues(optionasData)
					     .title("")
					     .buildSingle();
	gLayout1->addWidget(tx2FrontendGain, 1, 2);

	tx2FrontendGain->setUItoDataConversion([this, txFrontendGainOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, txFrontendGainOptions);
	});
	tx2FrontendGain->setDataToUIConversion([this, txFrontendGainOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, txFrontendGainOptions);
	});

	// adi,txmon-1-lo-cm

	IIOWidget *tx1LoCommonMode = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,txmon-1-lo-cm")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .optionsValues("[0 1 63]")
					     .title("")
					     .infoMessage("Please see the manual")
					     .buildSingle();
	gLayout1->addWidget(tx1LoCommonMode, 2, 1);

	// adi,txmon-2-lo-cm

	IIOWidget *tx2LoCommonMode = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,txmon-2-lo-cm")
					     .uiStrategy(IIOWidgetBuilder::RangeUi)
					     .optionsValues("[0 1 63]")
					     .title("")
					     .infoMessage("Please see the manual")
					     .buildSingle();
	gLayout1->addWidget(tx2LoCommonMode, 2, 2);

	layout->addLayout(gLayout1);

	// adi,txmon-low-high-thresh

	IIOWidget *lowHighThresh = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,txmon-low-high-thresh")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .optionsValues("[0 250 63750]")
					   .title("Low/High Gain Threshold (mdB)")
					   .infoMessage("Please see the manual")
					   .buildSingle();
	layout->addWidget(lowHighThresh);

	// adi,txmon-low-gain

	IIOWidget *lowGain = IIOWidgetBuilder(widget)
				     .device(m_device)
				     .attribute("adi,txmon-low-gain")
				     .uiStrategy(IIOWidgetBuilder::RangeUi)
				     .optionsValues("[0 1 31]")
				     .title("Low Gain (dB)")
				     .infoMessage("Please see the manual")
				     .buildSingle();
	layout->addWidget(lowGain);

	// adi,txmon-high-gain

	IIOWidget *highGain = IIOWidgetBuilder(widget)
				      .device(m_device)
				      .attribute("adi,txmon-high-gain")
				      .uiStrategy(IIOWidgetBuilder::RangeUi)
				      .optionsValues("[0 1 31]")
				      .title("High Gain (dB)")
				      .infoMessage("Please see the manual")
				      .buildSingle();
	layout->addWidget(highGain);

	// adi,txmon-delay

	IIOWidget *delay = IIOWidgetBuilder(widget)
				   .device(m_device)
				   .attribute("adi,txmon-delay")
				   .uiStrategy(IIOWidgetBuilder::RangeUi)
				   .optionsValues("[0 1 255]")
				   .title("Delay (RX samples)")
				   .infoMessage("Please see the manual")
				   .buildSingle();
	layout->addWidget(delay);

	// adi,txmon-duration

	IIOWidget *duration = IIOWidgetBuilder(widget)
				      .device(m_device)
				      .attribute("adi,txmon-duration")
				      .uiStrategy(IIOWidgetBuilder::RangeUi)
				      .optionsValues("[16 16 8192]")
				      .title("Duration (RX Samples)")
				      .infoMessage("Please see the manual")
				      .buildSingle();
	layout->addWidget(duration);

	// adi,txmon-dc-tracking-enable
	IIOWidget *dcTrackingEnabled = IIOWidgetBuilder(widget)
					       .device(m_device)
					       .attribute("adi,txmon-dc-tracking-enable")
					       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					       .title("Enable DC Tracking")
					       .infoMessage("Please see the manual")
					       .buildSingle();
	layout->addWidget(dcTrackingEnabled);
	dcTrackingEnabled->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	dcTrackingEnabled->showProgressBar(false);

	// adi,txmon-one-shot-mode-enable
	IIOWidget *oneShotModeEnabled = IIOWidgetBuilder(widget)
						.device(m_device)
						.attribute("adi,txmon-one-shot-mode-enable")
						.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						.title("Enable One Shot Mode")
						.infoMessage("Please see the manual")
						.buildSingle();
	layout->addWidget(oneShotModeEnabled);
	oneShotModeEnabled->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	oneShotModeEnabled->showProgressBar(false);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &TxMonitorWidget::readRequested, this, [=, this]() {
		tx1FrontendGain->readAsync();
		tx2FrontendGain->readAsync();
		tx1LoCommonMode->readAsync();
		tx2LoCommonMode->readAsync();
		lowHighThresh->readAsync();
		lowGain->readAsync();
		highGain->readAsync();
		delay->readAsync();
		duration->readAsync();
		dcTrackingEnabled->readAsync();
		oneShotModeEnabled->readAsync();
	});
}

TxMonitorWidget::~TxMonitorWidget() {}
