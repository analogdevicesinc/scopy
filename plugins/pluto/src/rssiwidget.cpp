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

#include "rssiwidget.h"

#include <style.h>
#include <iioutil/connectionprovider.h>
#include <iiowidgetbuilder.h>
#include <iiowidgetutils.h>

using namespace scopy;
using namespace pluto;

RssiWidget::RssiWidget(QString uri, QWidget *parent)
	: m_uri(uri)
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

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	QLabel *title = new QLabel("RSSI", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// adi,rssi-duration
	IIOWidget *rssiDuration = IIOWidgetBuilder(widget)
					  .device(m_device)
					  .attribute("adi,rssi-duration")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .optionsValues("[0 1 100000]")
					  .title("Duration (us)")
					  .infoMessage("Total RSSI measurement duration")
					  .buildSingle();
	layout->addWidget(rssiDuration);

	// adi,rssi-delay
	IIOWidget *rssiDelay =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,rssi-delay")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 100000]")
			.title("Delay (us)")
			.infoMessage(
				"When the RSSI algorithm (re)starts, the AD9361 first waits for the Rx signal path to "
				"settle. This delay is the “RSSI Delay”")
			.buildSingle();
	layout->addWidget(rssiDelay);

	// wait  adi,rssi-wait
	IIOWidget *rssiWait = IIOWidgetBuilder(widget)
				      .device(m_device)
				      .attribute("adi,rssi-wait")
				      .uiStrategy(IIOWidgetBuilder::RangeUi)
				      .optionsValues("[0 1 100000]")
				      .title("Wait (us)")
				      .infoMessage("After the “RSSI Delay” the RSSI algorithm alternates between "
						   "measuring RSSI and waiting "
						   "“RSSI Wait” to measure RSSI")
				      .buildSingle();
	layout->addWidget(rssiWait);

	// adi,rssi-restart-mode
	QMap<QString, QString> *rssiRestartModeOptions = new QMap<QString, QString>();
	rssiRestartModeOptions->insert("0", "AGC_in_Fast_Attack_Mode_Locks_the_Gain");
	rssiRestartModeOptions->insert("1", "EN_AGC_pin_is_pulled_High");
	rssiRestartModeOptions->insert("2", "AD9361_Enters_Rx_Mode");
	rssiRestartModeOptions->insert("3", "Gain_Change_Occurs");
	rssiRestartModeOptions->insert("4", "SPI_Write_to_Register");
	rssiRestartModeOptions->insert("5", "Gain_Change_Occurs_OR_EN_AGC_pin_pulled_High");

	auto values = rssiRestartModeOptions->values();
	QString optionasData = "";
	for(int i = 0; i < values.size(); i++) {
		optionasData += " " + values.at(i);
	}

	IIOWidget *rssiRestartMode = IIOWidgetBuilder(widget)
					     .device(m_device)
					     .attribute("adi,rssi-restart-mode")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .optionsValues(optionasData)
					     .title("Restart Mode")
					     .buildSingle();
	layout->addWidget(rssiRestartMode);

	rssiRestartMode->setUItoDataConversion([this, rssiRestartModeOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, rssiRestartModeOptions);
	});
	rssiRestartMode->setDataToUIConversion([this, rssiRestartModeOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, rssiRestartModeOptions);
	});

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &RssiWidget::readRequested, this, [=, this]() {
		rssiDuration->readAsync();
		rssiDelay->readAsync();
		rssiWait->readAsync();
		rssiRestartMode->readAsync();
	});
}

RssiWidget::~RssiWidget() { ConnectionProvider::close(m_uri); }
