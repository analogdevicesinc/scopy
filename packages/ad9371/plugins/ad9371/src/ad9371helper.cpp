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

#include "ad9371helper.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <style.h>
#include <QPushButton>
#include <QLoggingCategory>
#include <QTimer>
#include <QDoubleSpinBox>
#include <cmath>

Q_LOGGING_CATEGORY(CAT_AD9371HELPER, "AD9371Helper");

using namespace scopy;
using namespace ad9371;

// DPD status enum-to-string mapping (from ad9371.c:207-220)
static const QStringList dpdStatusStrings = {
	"No Error",
	"Error: ORx disabled",
	"Error: Tx disabled",
	"Error: DPD initialization not run",
	"Error: Path delay not setup",
	"Error: ORx signal too low",
	"Error: ORx signal saturated",
	"Error: Tx signal too low",
	"Error: Tx signal saturated",
	"Error: Model error high",
	"Error: AM AM outliers",
	"Error: Invalid Tx profile",
	"Error: ORx QEC Disabled",
};

// CLGC status enum-to-string mapping (from ad9371.c:184-198)
static const QStringList clgcStatusStrings = {
	"No Error",
	"Error: TX is disabled",
	"Error: ORx is disabled",
	"Error: Loopback switch is closed",
	"Error: Data measurement aborted during capture",
	"Error: No initial calibration was done",
	"Error: Path delay not setup",
	"Error: No apply control is possible",
	"Error: Control value is out of range",
	"Error: CLGC feature is disabled",
	"Error: TX attenuation is capped",
	"Error: Gain measurement",
	"Error: No GPIO configured in single ORx configuration",
	"Error: Tx is not observable with any of the ORx Channels",
};

// VSWR status enum-to-string mapping (from ad9371.c:222-233)
static const QStringList vswrStatusStrings = {
	"No Error",
	"Error: TX disabled",
	"Error: ORx disabled",
	"Error: Loopback switch is closed",
	"Error: No initial calibration was done",
	"Error: Path delay not setup",
	"Error: Data capture aborted",
	"Error: VSWR is disabled",
	"Error: Entering Cal",
	"Error: No GPIO configured in single ORx configuration",
	"Error: Tx is not observable with any of the ORx Channels",
};

AD9371Helper::AD9371Helper(IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_group(group)
{}

QWidget *AD9371Helper::generateGlobalSettingsWidget(iio_device *dev, bool has_dpd, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	// Row 1: ENSM Mode | CAL RX QEC | CAL TX QEC | CAL TX LOL | CAL TX LOL EXT
	QHBoxLayout *calRow1 = new QHBoxLayout();

	// #1 — ensm_mode (ComboUi, device-level)
	IIOWidget *ensmMode = IIOWidgetBuilder(widget)
				      .device(dev)
				      .attribute("ensm_mode")
				      .optionsAttribute("ensm_mode_available")
				      .title("ENSM Mode")
				      .uiStrategy(IIOWidgetBuilder::ComboUi)
				      .group(m_group)
				      .buildSingle();
	calRow1->addWidget(ensmMode);
	connect(this, &AD9371Helper::readRequested, ensmMode, &IIOWidget::readAsync);

	// #5 — calibrate_rx_qec_en (CheckBoxUi, always visible)
	IIOWidget *calibrateRxQecEn = IIOWidgetBuilder(widget)
					      .device(dev)
					      .attribute("calibrate_rx_qec_en")
					      .title("Calibrate RX QEC")
					      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					      .group(m_group)
					      .buildSingle();
	calRow1->addWidget(calibrateRxQecEn);
	connect(this, &AD9371Helper::readRequested, calibrateRxQecEn, &IIOWidget::readAsync);

	// #6 — calibrate_tx_qec_en (CheckBoxUi, always visible)
	IIOWidget *calibrateTxQecEn = IIOWidgetBuilder(widget)
					      .device(dev)
					      .attribute("calibrate_tx_qec_en")
					      .title("Calibrate TX QEC")
					      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					      .group(m_group)
					      .buildSingle();
	calRow1->addWidget(calibrateTxQecEn);
	connect(this, &AD9371Helper::readRequested, calibrateTxQecEn, &IIOWidget::readAsync);

	// #7 — calibrate_tx_lol_en (CheckBoxUi, always visible)
	IIOWidget *calibrateTxLolEn = IIOWidgetBuilder(widget)
					      .device(dev)
					      .attribute("calibrate_tx_lol_en")
					      .title("Calibrate TX LOL")
					      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					      .group(m_group)
					      .buildSingle();
	calRow1->addWidget(calibrateTxLolEn);
	connect(this, &AD9371Helper::readRequested, calibrateTxLolEn, &IIOWidget::readAsync);

	// #8 — calibrate_tx_lol_ext_en (CheckBoxUi, always visible)
	IIOWidget *calibrateTxLolExtEn = IIOWidgetBuilder(widget)
						 .device(dev)
						 .attribute("calibrate_tx_lol_ext_en")
						 .title("Calibrate TX LOL EXT")
						 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						 .group(m_group)
						 .buildSingle();
	calRow1->addWidget(calibrateTxLolExtEn);
	connect(this, &AD9371Helper::readRequested, calibrateTxLolExtEn, &IIOWidget::readAsync);

	layout->addLayout(calRow1);

	// Row 2: DPD/CLGC/VSWR checkboxes (conditional on has_dpd)
	if(has_dpd) {
		QHBoxLayout *calRow2 = new QHBoxLayout();

		// #2 — calibrate_dpd_en
		IIOWidget *calibrateDpdEn = IIOWidgetBuilder(widget)
						    .device(dev)
						    .attribute("calibrate_dpd_en")
						    .title("Calibrate DPD")
						    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						    .group(m_group)
						    .buildSingle();
		calRow2->addWidget(calibrateDpdEn);
		connect(this, &AD9371Helper::readRequested, calibrateDpdEn, &IIOWidget::readAsync);

		// #3 — calibrate_clgc_en
		IIOWidget *calibrateClgcEn = IIOWidgetBuilder(widget)
						     .device(dev)
						     .attribute("calibrate_clgc_en")
						     .title("Calibrate CLGC")
						     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						     .group(m_group)
						     .buildSingle();
		calRow2->addWidget(calibrateClgcEn);
		connect(this, &AD9371Helper::readRequested, calibrateClgcEn, &IIOWidget::readAsync);

		// #4 — calibrate_vswr_en
		IIOWidget *calibrateVswrEn = IIOWidgetBuilder(widget)
						     .device(dev)
						     .attribute("calibrate_vswr_en")
						     .title("Calibrate VSWR")
						     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						     .group(m_group)
						     .buildSingle();
		calRow2->addWidget(calibrateVswrEn);
		connect(this, &AD9371Helper::readRequested, calibrateVswrEn, &IIOWidget::readAsync);

		layout->addLayout(calRow2);
	}

	// #9 — calibrate (QPushButton, write-only)
	QPushButton *calibrateBtn = new QPushButton("Calibrate", widget);
	connect(calibrateBtn, &QPushButton::clicked, this, [dev]() { iio_device_attr_write(dev, "calibrate", "1"); });
	layout->addWidget(calibrateBtn);

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}

QWidget *AD9371Helper::generateRxChainWidget(iio_device *dev, iio_device *cap, bool is_2rx_2tx, QString title,
					     QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	iio_channel *ch0 = iio_device_find_channel(dev, "voltage0", false);
	iio_channel *altVoltage0 = iio_device_find_channel(dev, "altvoltage0", true);
	const char *rxLoFreqAttr = (altVoltage0 && iio_channel_find_attr(altVoltage0, "frequency")) ? "frequency" : "RX_LO_frequency";

	// Header row: RF Bandwidth (RO) | Sampling Rate | RX LO Frequency | Gain Control Mode
	QHBoxLayout *headerRow = new QHBoxLayout();

	// Read-only: RF Bandwidth (Template F — ÷1e6, MHz)
	IIOWidget *rfBandwidth = IIOWidgetBuilder(widget)
					 .channel(ch0)
					 .attribute("rf_bandwidth")
					 .title("RF Bandwidth (MHz)")
					 .group(m_group)
					 .buildSingle();
	rfBandwidth->setEnabled(false);
	rfBandwidth->setDataToUIConversion([](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 2); });
	headerRow->addWidget(rfBandwidth);
	QTimer *rfBwTimer = new QTimer(widget);
	connect(rfBwTimer, &QTimer::timeout, rfBandwidth, &IIOWidget::readAsync);
	rfBwTimer->start(1000);
	connect(this, &AD9371Helper::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// sampling_frequency (RangeUi, channel-level, MHz conversion)
	IIOWidget *samplingFrequency = IIOWidgetBuilder(widget)
					       .channel(ch0)
					       .attribute("sampling_frequency")
					       .title("Sampling Frequency(MHz)")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .group(m_group)
					       .buildSingle();
	samplingFrequency->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	samplingFrequency->setRangeToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	samplingFrequency->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	headerRow->addWidget(samplingFrequency);
	connect(this, &AD9371Helper::readRequested, samplingFrequency, &IIOWidget::readAsync);

	// RX LO frequency (RangeUi, altvoltage0 channel, MHz conversion)
	IIOWidget *rxLoFrequency = IIOWidgetBuilder(widget)
					   .channel(altVoltage0)
					   .attribute(rxLoFreqAttr)
					   .optionsAttribute("frequency_available")
					   .title("RX LO Frequency(MHz)")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .group(m_group)
					   .buildSingle();
	rxLoFrequency->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	rxLoFrequency->setRangeToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	rxLoFrequency->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	headerRow->addWidget(rxLoFrequency);
	connect(this, &AD9371Helper::readRequested, rxLoFrequency, &IIOWidget::readAsync);

	// gain_control_mode (ComboUi, channel-level)
	IIOWidget *gainControlMode = IIOWidgetBuilder(widget)
					     .channel(ch0)
					     .attribute("gain_control_mode")
					     .optionsAttribute("gain_control_mode_available")
					     .title("Gain Control Mode")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .group(m_group)
					     .buildSingle();
	headerRow->addWidget(gainControlMode);
	connect(this, &AD9371Helper::readRequested, gainControlMode, &IIOWidget::readAsync);

	layout->addLayout(headerRow);

	// Side-by-side RX1 / RX2 columns
	QHBoxLayout *channelColumns = new QHBoxLayout();

	// RX1 column
	QWidget *rx1Container = new QWidget(widget);
	QVBoxLayout *rx1Layout = new QVBoxLayout(rx1Container);
	rx1Container->setLayout(rx1Layout);
	Style::setStyle(rx1Container, style::properties::widget::border_interactive);

	QLabel *rx1Label = new QLabel("RX 1", rx1Container);
	Style::setStyle(rx1Label, style::properties::label::menuBig);
	rx1Layout->addWidget(rx1Label);

	// RX1 — hardwaregain
	IIOWidget *hardwaregain = IIOWidgetBuilder(rx1Container)
					  .channel(ch0)
					  .attribute("hardwaregain")
					  .optionsAttribute("hardwaregain_available")
					  .title("Hardware Gain(dB)")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .group(m_group)
					  .buildSingle();
	hardwaregain->setDataToUIConversion([](QString data) {
		auto result = data.split(" ");
		return result.first();
	});
	rx1Layout->addWidget(hardwaregain);
	connect(this, &AD9371Helper::readRequested, hardwaregain, &IIOWidget::readAsync);

	// RX1 — RSSI read-only
	IIOWidget *rssi = IIOWidgetBuilder(rx1Container)
				  .channel(ch0)
				  .attribute("rssi")
				  .title("RSSI (dB)")
				  .group(m_group)
				  .buildSingle();
	rssi->setEnabled(false);
	rx1Layout->addWidget(rssi);
	QTimer *rssiTimer = new QTimer(widget);
	connect(rssiTimer, &QTimer::timeout, rssi, &IIOWidget::readAsync);
	rssiTimer->start(1000);
	connect(this, &AD9371Helper::readRequested, rssi, &IIOWidget::readAsync);

	// RX1 — temp_comp_gain
	IIOWidget *tempCompGain = IIOWidgetBuilder(rx1Container)
					  .channel(ch0)
					  .attribute("temp_comp_gain")
					  .title("Temp Comp Gain")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .group(m_group)
					  .buildSingle();
	rx1Layout->addWidget(tempCompGain);
	connect(this, &AD9371Helper::readRequested, tempCompGain, &IIOWidget::readAsync);

	// RX1 — quadrature_tracking_en
	IIOWidget *quadratureTrackingEn = IIOWidgetBuilder(rx1Container)
						  .channel(ch0)
						  .attribute("quadrature_tracking_en")
						  .title("Quadrature Tracking")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .group(m_group)
						  .buildSingle();
	rx1Layout->addWidget(quadratureTrackingEn);
	connect(this, &AD9371Helper::readRequested, quadratureTrackingEn, &IIOWidget::readAsync);

	rx1Layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	channelColumns->addWidget(rx1Container);

	// RX2 column (conditional on is_2rx_2tx)
	if(is_2rx_2tx) {
		iio_channel *ch1 = iio_device_find_channel(dev, "voltage1", false);
		if(ch1) {
			QWidget *rx2Container = new QWidget(widget);
			QVBoxLayout *rx2Layout = new QVBoxLayout(rx2Container);
			rx2Container->setLayout(rx2Layout);
			Style::setStyle(rx2Container, style::properties::widget::border_interactive);

			QLabel *rx2Label = new QLabel("RX 2", rx2Container);
			Style::setStyle(rx2Label, style::properties::label::menuBig);
			rx2Layout->addWidget(rx2Label);

			// RX2 — hardwaregain
			IIOWidget *hardwaregainRx2 = IIOWidgetBuilder(rx2Container)
							     .channel(ch1)
							     .attribute("hardwaregain")
							     .optionsAttribute("hardwaregain_available")
							     .title("Hardware Gain(dB)")
							     .uiStrategy(IIOWidgetBuilder::RangeUi)
							     .group(m_group)
							     .buildSingle();
			hardwaregainRx2->setDataToUIConversion([](QString data) {
				auto result = data.split(" ");
				return result.first();
			});
			rx2Layout->addWidget(hardwaregainRx2);
			connect(this, &AD9371Helper::readRequested, hardwaregainRx2, &IIOWidget::readAsync);

			// RX2 — RSSI read-only
			IIOWidget *rssiRx2 = IIOWidgetBuilder(rx2Container)
						     .channel(ch1)
						     .attribute("rssi")
						     .title("RSSI (dB)")
						     .group(m_group)
						     .buildSingle();
			rssiRx2->setEnabled(false);
			rx2Layout->addWidget(rssiRx2);
			QTimer *rssiRx2Timer = new QTimer(widget);
			connect(rssiRx2Timer, &QTimer::timeout, rssiRx2, &IIOWidget::readAsync);
			rssiRx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, rssiRx2, &IIOWidget::readAsync);

			// RX2 — temp_comp_gain
			IIOWidget *tempCompGainRx2 = IIOWidgetBuilder(rx2Container)
							     .channel(ch1)
							     .attribute("temp_comp_gain")
							     .title("Temp Comp Gain")
							     .uiStrategy(IIOWidgetBuilder::RangeUi)
							     .group(m_group)
							     .buildSingle();
			rx2Layout->addWidget(tempCompGainRx2);
			connect(this, &AD9371Helper::readRequested, tempCompGainRx2, &IIOWidget::readAsync);

			// RX2 — quadrature_tracking_en
			IIOWidget *quadTrackRx2 = IIOWidgetBuilder(rx2Container)
							  .channel(ch1)
							  .attribute("quadrature_tracking_en")
							  .title("Quadrature Tracking")
							  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
							  .group(m_group)
							  .buildSingle();
			rx2Layout->addWidget(quadTrackRx2);
			connect(this, &AD9371Helper::readRequested, quadTrackRx2, &IIOWidget::readAsync);

			rx2Layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
			channelColumns->addWidget(rx2Container);
		}
	}

	channelColumns->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));
	layout->addLayout(channelColumns);

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}

QWidget *AD9371Helper::generateObsRxChainWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	iio_channel *ch2 = iio_device_find_channel(dev, "voltage2", false);
	iio_channel *altVoltage2 = iio_device_find_channel(dev, "altvoltage2", true);
	const char *snLoFreqAttr = (altVoltage2 && iio_channel_find_attr(altVoltage2, "frequency")) ? "frequency" : "RX_SN_LO_frequency";

	// Header row: RF Bandwidth (RO) | SN LO Frequency | RF Port Select
	QHBoxLayout *headerRow = new QHBoxLayout();

	// Read-only: RF Bandwidth (Template F — ÷1e6, MHz)
	IIOWidget *rfBandwidth = IIOWidgetBuilder(widget)
					 .channel(ch2)
					 .attribute("rf_bandwidth")
					 .title("RF Bandwidth (MHz)")
					 .group(m_group)
					 .buildSingle();
	rfBandwidth->setEnabled(false);
	rfBandwidth->setDataToUIConversion([](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 2); });
	headerRow->addWidget(rfBandwidth);
	QTimer *rfBwTimer = new QTimer(widget);
	connect(rfBwTimer, &QTimer::timeout, rfBandwidth, &IIOWidget::readAsync);
	rfBwTimer->start(1000);
	connect(this, &AD9371Helper::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// SN LO frequency (RangeUi, altvoltage2 channel, MHz conversion)
	IIOWidget *snLoFrequency = IIOWidgetBuilder(widget)
					   .channel(altVoltage2)
					   .attribute(snLoFreqAttr)
					   .optionsAttribute("frequency_available")
					   .title("SN LO Frequency(MHz)")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .group(m_group)
					   .buildSingle();
	snLoFrequency->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	snLoFrequency->setRangeToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	snLoFrequency->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	headerRow->addWidget(snLoFrequency);
	connect(this, &AD9371Helper::readRequested, snLoFrequency, &IIOWidget::readAsync);

	// rf_port_select (ComboUi, channel-level, OBS-specific)
	IIOWidget *rfPortSelect = IIOWidgetBuilder(widget)
					  .channel(ch2)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .title("RF Port Select")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .group(m_group)
					  .buildSingle();
	headerRow->addWidget(rfPortSelect);
	connect(this, &AD9371Helper::readRequested, rfPortSelect, &IIOWidget::readAsync);

	layout->addLayout(headerRow);

	// OBS container with per-channel widgets
	QWidget *obsContainer = new QWidget(widget);
	QVBoxLayout *obsLayout = new QVBoxLayout(obsContainer);
	obsContainer->setLayout(obsLayout);
	Style::setStyle(obsContainer, style::properties::widget::border_interactive);

	QLabel *obsLabel = new QLabel("OBS", obsContainer);
	Style::setStyle(obsLabel, style::properties::label::menuBig);
	obsLayout->addWidget(obsLabel);

	// OBS — hardwaregain
	IIOWidget *hardwaregain = IIOWidgetBuilder(obsContainer)
					  .channel(ch2)
					  .attribute("hardwaregain")
					  .optionsAttribute("hardwaregain_available")
					  .title("Hardware Gain(dB)")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .group(m_group)
					  .buildSingle();
	hardwaregain->setDataToUIConversion([](QString data) {
		auto result = data.split(" ");
		return result.first();
	});
	obsLayout->addWidget(hardwaregain);
	connect(this, &AD9371Helper::readRequested, hardwaregain, &IIOWidget::readAsync);

	// OBS — RSSI read-only
	IIOWidget *rssi = IIOWidgetBuilder(obsContainer)
				  .channel(ch2)
				  .attribute("rssi")
				  .title("RSSI (dB)")
				  .group(m_group)
				  .buildSingle();
	rssi->setEnabled(false);
	obsLayout->addWidget(rssi);
	QTimer *rssiTimer = new QTimer(widget);
	connect(rssiTimer, &QTimer::timeout, rssi, &IIOWidget::readAsync);
	rssiTimer->start(1000);
	connect(this, &AD9371Helper::readRequested, rssi, &IIOWidget::readAsync);

	// OBS — gain_control_mode
	IIOWidget *gainControlMode = IIOWidgetBuilder(obsContainer)
					     .channel(ch2)
					     .attribute("gain_control_mode")
					     .optionsAttribute("gain_control_mode_available")
					     .title("Gain Control Mode")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .group(m_group)
					     .buildSingle();
	obsLayout->addWidget(gainControlMode);
	connect(this, &AD9371Helper::readRequested, gainControlMode, &IIOWidget::readAsync);

	// OBS — temp_comp_gain
	IIOWidget *tempCompGain = IIOWidgetBuilder(obsContainer)
					  .channel(ch2)
					  .attribute("temp_comp_gain")
					  .title("Temp Comp Gain")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .group(m_group)
					  .buildSingle();
	obsLayout->addWidget(tempCompGain);
	connect(this, &AD9371Helper::readRequested, tempCompGain, &IIOWidget::readAsync);

	// OBS — quadrature_tracking_en
	IIOWidget *quadratureTrackingEn = IIOWidgetBuilder(obsContainer)
						  .channel(ch2)
						  .attribute("quadrature_tracking_en")
						  .title("Quadrature Tracking")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .group(m_group)
						  .buildSingle();
	obsLayout->addWidget(quadratureTrackingEn);
	connect(this, &AD9371Helper::readRequested, quadratureTrackingEn, &IIOWidget::readAsync);

	obsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	layout->addWidget(obsContainer);

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}

QWidget *AD9371Helper::generateTxChainWidget(iio_device *dev, bool has_dpd, bool is_2rx_2tx, QString title,
					     QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	iio_channel *ch0 = iio_device_find_channel(dev, "voltage0", true);
	iio_channel *altVoltage1 = iio_device_find_channel(dev, "altvoltage1", true);
	const char *txLoFreqAttr = (altVoltage1 && iio_channel_find_attr(altVoltage1, "frequency")) ? "frequency" : "TX_LO_frequency";

	// Header row: RF Bandwidth (RO) | Sampling Rate | TX LO Frequency
	QHBoxLayout *headerRow = new QHBoxLayout();

	// Read-only: RF Bandwidth (Template F — ÷1e6, MHz)
	IIOWidget *rfBandwidth = IIOWidgetBuilder(widget)
					 .channel(ch0)
					 .attribute("rf_bandwidth")
					 .title("RF Bandwidth (MHz)")
					 .group(m_group)
					 .buildSingle();
	rfBandwidth->setEnabled(false);
	rfBandwidth->setDataToUIConversion([](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 2); });
	headerRow->addWidget(rfBandwidth);
	QTimer *rfBwTimer = new QTimer(widget);
	connect(rfBwTimer, &QTimer::timeout, rfBandwidth, &IIOWidget::readAsync);
	rfBwTimer->start(1000);
	connect(this, &AD9371Helper::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// sampling_frequency (RangeUi, channel-level, MHz conversion)
	IIOWidget *samplingFrequency = IIOWidgetBuilder(widget)
					       .channel(ch0)
					       .attribute("sampling_frequency")
					       .title("Sampling Frequency(MHz)")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .group(m_group)
					       .buildSingle();
	samplingFrequency->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	samplingFrequency->setRangeToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	samplingFrequency->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	headerRow->addWidget(samplingFrequency);
	connect(this, &AD9371Helper::readRequested, samplingFrequency, &IIOWidget::readAsync);

	// TX LO frequency (RangeUi, altvoltage1 channel, MHz conversion)
	IIOWidget *txLoFrequency = IIOWidgetBuilder(widget)
					   .channel(altVoltage1)
					   .attribute(txLoFreqAttr)
					   .optionsAttribute("frequency_available")
					   .title("TX LO Frequency(MHz)")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .group(m_group)
					   .buildSingle();
	txLoFrequency->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	txLoFrequency->setRangeToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	txLoFrequency->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	headerRow->addWidget(txLoFrequency);
	connect(this, &AD9371Helper::readRequested, txLoFrequency, &IIOWidget::readAsync);

	layout->addLayout(headerRow);

	// Side-by-side TX1 / TX2 columns
	QHBoxLayout *channelColumns = new QHBoxLayout();

	// TX1 column
	QWidget *tx1Container = new QWidget(widget);
	QVBoxLayout *tx1Layout = new QVBoxLayout(tx1Container);
	tx1Container->setLayout(tx1Layout);
	Style::setStyle(tx1Container, style::properties::widget::border_interactive);

	QLabel *tx1Label = new QLabel("TX 1", tx1Container);
	Style::setStyle(tx1Label, style::properties::label::menuBig);
	tx1Layout->addWidget(tx1Label);

	// TX1 — hardwaregain (Attenuation)
	IIOWidget *hardwaregain = IIOWidgetBuilder(tx1Container)
					  .channel(ch0)
					  .attribute("hardwaregain")
					  .optionsAttribute("hardwaregain_available")
					  .title("Attenuation(dB)")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .group(m_group)
					  .buildSingle();
	hardwaregain->setDataToUIConversion([](QString data) {
		auto result = data.split(" ");
		return result.first();
	});
	tx1Layout->addWidget(hardwaregain);
	connect(this, &AD9371Helper::readRequested, hardwaregain, &IIOWidget::readAsync);

	// TX1 — quadrature_tracking_en
	IIOWidget *quadratureTrackingEn = IIOWidgetBuilder(tx1Container)
						  .channel(ch0)
						  .attribute("quadrature_tracking_en")
						  .title("Quadrature Tracking")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .group(m_group)
						  .buildSingle();
	tx1Layout->addWidget(quadratureTrackingEn);
	connect(this, &AD9371Helper::readRequested, quadratureTrackingEn, &IIOWidget::readAsync);

	// TX1 — lo_leakage_tracking_en
	IIOWidget *loLeakageTrackingEn = IIOWidgetBuilder(tx1Container)
						 .channel(ch0)
						 .attribute("lo_leakage_tracking_en")
						 .title("LO Leakage Tracking")
						 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						 .group(m_group)
						 .buildSingle();
	tx1Layout->addWidget(loLeakageTrackingEn);
	connect(this, &AD9371Helper::readRequested, loLeakageTrackingEn, &IIOWidget::readAsync);

	tx1Layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	channelColumns->addWidget(tx1Container);

	// TX2 column (conditional on is_2rx_2tx)
	if(is_2rx_2tx) {
		iio_channel *ch1 = iio_device_find_channel(dev, "voltage1", true);
		if(ch1) {
			QWidget *tx2Container = new QWidget(widget);
			QVBoxLayout *tx2Layout = new QVBoxLayout(tx2Container);
			tx2Container->setLayout(tx2Layout);
			Style::setStyle(tx2Container, style::properties::widget::border_interactive);

			QLabel *tx2Label = new QLabel("TX 2", tx2Container);
			Style::setStyle(tx2Label, style::properties::label::menuBig);
			tx2Layout->addWidget(tx2Label);

			// TX2 — hardwaregain (Attenuation)
			IIOWidget *hardwaregainTx2 = IIOWidgetBuilder(tx2Container)
							     .channel(ch1)
							     .attribute("hardwaregain")
							     .optionsAttribute("hardwaregain_available")
							     .title("Attenuation(dB)")
							     .uiStrategy(IIOWidgetBuilder::RangeUi)
							     .group(m_group)
							     .buildSingle();
			hardwaregainTx2->setDataToUIConversion([](QString data) {
				auto result = data.split(" ");
				return result.first();
			});
			tx2Layout->addWidget(hardwaregainTx2);
			connect(this, &AD9371Helper::readRequested, hardwaregainTx2, &IIOWidget::readAsync);

			// TX2 — quadrature_tracking_en
			IIOWidget *quadTrackTx2 = IIOWidgetBuilder(tx2Container)
							  .channel(ch1)
							  .attribute("quadrature_tracking_en")
							  .title("Quadrature Tracking")
							  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
							  .group(m_group)
							  .buildSingle();
			tx2Layout->addWidget(quadTrackTx2);
			connect(this, &AD9371Helper::readRequested, quadTrackTx2, &IIOWidget::readAsync);

			// TX2 — lo_leakage_tracking_en
			IIOWidget *loLeakageTx2 = IIOWidgetBuilder(tx2Container)
							  .channel(ch1)
							  .attribute("lo_leakage_tracking_en")
							  .title("LO Leakage Tracking")
							  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
							  .group(m_group)
							  .buildSingle();
			tx2Layout->addWidget(loLeakageTx2);
			connect(this, &AD9371Helper::readRequested, loLeakageTx2, &IIOWidget::readAsync);

			tx2Layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
			channelColumns->addWidget(tx2Container);
		}
	}

	channelColumns->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));
	layout->addLayout(channelColumns);

	if(has_dpd) {
		iio_channel *ch1 = iio_device_find_channel(dev, "voltage1", true);

		// --- DPD ---

		// dpd_tracking_en TX1
		IIOWidget *dpdTrackingEnTx1 = IIOWidgetBuilder(widget)
						      .channel(ch0)
						      .attribute("dpd_tracking_en")
						      .title("DPD Tracking TX1")
						      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						      .group(m_group)
						      .buildSingle();
		layout->addWidget(dpdTrackingEnTx1);
		connect(this, &AD9371Helper::readRequested, dpdTrackingEnTx1, &IIOWidget::readAsync);

		if(ch1) {
			// dpd_tracking_en TX2
			IIOWidget *dpdTrackingEnTx2 = IIOWidgetBuilder(widget)
							      .channel(ch1)
							      .attribute("dpd_tracking_en")
							      .title("DPD Tracking TX2")
							      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
							      .group(m_group)
							      .buildSingle();
			layout->addWidget(dpdTrackingEnTx2);
			connect(this, &AD9371Helper::readRequested, dpdTrackingEnTx2, &IIOWidget::readAsync);
		}

		// dpd_actuator_en TX1
		IIOWidget *dpdActuatorEnTx1 = IIOWidgetBuilder(widget)
						      .channel(ch0)
						      .attribute("dpd_actuator_en")
						      .title("DPD Actuator TX1")
						      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						      .group(m_group)
						      .buildSingle();
		layout->addWidget(dpdActuatorEnTx1);
		connect(this, &AD9371Helper::readRequested, dpdActuatorEnTx1, &IIOWidget::readAsync);

		if(ch1) {
			// dpd_actuator_en TX2
			IIOWidget *dpdActuatorEnTx2 = IIOWidgetBuilder(widget)
							      .channel(ch1)
							      .attribute("dpd_actuator_en")
							      .title("DPD Actuator TX2")
							      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
							      .group(m_group)
							      .buildSingle();
			layout->addWidget(dpdActuatorEnTx2);
			connect(this, &AD9371Helper::readRequested, dpdActuatorEnTx2, &IIOWidget::readAsync);
		}

		// dpd_reset_en TX1 (button, write-only)
		QPushButton *dpdResetTx1 = new QPushButton("DPD Reset TX1", widget);
		connect(dpdResetTx1, &QPushButton::clicked, this,
			[ch0]() { iio_channel_attr_write(ch0, "dpd_reset_en", "1"); });
		layout->addWidget(dpdResetTx1);

		if(ch1) {
			// dpd_reset_en TX2 (button, write-only)
			QPushButton *dpdResetTx2 = new QPushButton("DPD Reset TX2", widget);
			connect(dpdResetTx2, &QPushButton::clicked, this,
				[ch1]() { iio_channel_attr_write(ch1, "dpd_reset_en", "1"); });
			layout->addWidget(dpdResetTx2);
		}

		// --- CLGC ---

		// clgc_tracking_en TX1
		IIOWidget *clgcTrackingEnTx1 = IIOWidgetBuilder(widget)
						       .channel(ch0)
						       .attribute("clgc_tracking_en")
						       .title("CLGC Tracking TX1")
						       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						       .group(m_group)
						       .buildSingle();
		layout->addWidget(clgcTrackingEnTx1);
		connect(this, &AD9371Helper::readRequested, clgcTrackingEnTx1, &IIOWidget::readAsync);

		if(ch1) {
			// clgc_tracking_en TX2
			IIOWidget *clgcTrackingEnTx2 = IIOWidgetBuilder(widget)
							       .channel(ch1)
							       .attribute("clgc_tracking_en")
							       .title("CLGC Tracking TX2")
							       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
							       .group(m_group)
							       .buildSingle();
			layout->addWidget(clgcTrackingEnTx2);
			connect(this, &AD9371Helper::readRequested, clgcTrackingEnTx2, &IIOWidget::readAsync);
		}

		// clgc_desired_gain TX1
		IIOWidget *clgcDesiredGainTx1 = IIOWidgetBuilder(widget)
							.channel(ch0)
							.attribute("clgc_desired_gain")
							.title("CLGC Desired Gain TX1")
							.uiStrategy(IIOWidgetBuilder::RangeUi)
							.group(m_group)
							.buildSingle();
		clgcDesiredGainTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		clgcDesiredGainTx1->setRangeToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		clgcDesiredGainTx1->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 100.0, 'f', 0); });
		layout->addWidget(clgcDesiredGainTx1);
		connect(this, &AD9371Helper::readRequested, clgcDesiredGainTx1, &IIOWidget::readAsync);

		if(ch1) {
			// clgc_desired_gain TX2
			IIOWidget *clgcDesiredGainTx2 = IIOWidgetBuilder(widget)
								.channel(ch1)
								.attribute("clgc_desired_gain")
								.title("CLGC Desired Gain TX2")
								.uiStrategy(IIOWidgetBuilder::RangeUi)
								.group(m_group)
								.buildSingle();
			clgcDesiredGainTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			clgcDesiredGainTx2->setRangeToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			clgcDesiredGainTx2->setUItoDataConversion(
				[](QString data) { return QString::number(data.toDouble() * 100.0, 'f', 0); });
			layout->addWidget(clgcDesiredGainTx2);
			connect(this, &AD9371Helper::readRequested, clgcDesiredGainTx2, &IIOWidget::readAsync);
		}

		// --- VSWR ---

		// vswr_tracking_en TX1
		IIOWidget *vswrTrackingEnTx1 = IIOWidgetBuilder(widget)
						       .channel(ch0)
						       .attribute("vswr_tracking_en")
						       .title("VSWR Tracking TX1")
						       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						       .group(m_group)
						       .buildSingle();
		layout->addWidget(vswrTrackingEnTx1);
		connect(this, &AD9371Helper::readRequested, vswrTrackingEnTx1, &IIOWidget::readAsync);

		if(ch1) {
			// vswr_tracking_en TX2
			IIOWidget *vswrTrackingEnTx2 = IIOWidgetBuilder(widget)
							       .channel(ch1)
							       .attribute("vswr_tracking_en")
							       .title("VSWR Tracking TX2")
							       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
							       .group(m_group)
							       .buildSingle();
			layout->addWidget(vswrTrackingEnTx2);
			connect(this, &AD9371Helper::readRequested, vswrTrackingEnTx2, &IIOWidget::readAsync);
		}
	}

	// Phase 3b-d: DPD/CLGC/VSWR read-only status labels (conditional on has_dpd)
	if(has_dpd) {
		iio_channel *ch1 = iio_device_find_channel(dev, "voltage1", true);

		// --- DPD Status Labels (Template D/F/G, timer 1000ms) ---

		// dpd_track_count TX1
		IIOWidget *dpdTrackCountTx1 = IIOWidgetBuilder(widget)
						      .channel(ch0)
						      .attribute("dpd_track_count")
						      .title("DPD Track Count (TX1)")
						      .group(m_group)
						      .buildSingle();
		dpdTrackCountTx1->setEnabled(false);
		layout->addWidget(dpdTrackCountTx1);
		QTimer *dpdTrackCountTx1Timer = new QTimer(widget);
		connect(dpdTrackCountTx1Timer, &QTimer::timeout, dpdTrackCountTx1, &IIOWidget::readAsync);
		dpdTrackCountTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, dpdTrackCountTx1, &IIOWidget::readAsync);

		// dpd_model_error TX1 (÷10, %)
		IIOWidget *dpdModelErrorTx1 = IIOWidgetBuilder(widget)
						      .channel(ch0)
						      .attribute("dpd_model_error")
						      .title("DPD Model Error (TX1) (%)")
						      .group(m_group)
						      .buildSingle();
		dpdModelErrorTx1->setEnabled(false);
		dpdModelErrorTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 10.0, 'f', 1); });
		layout->addWidget(dpdModelErrorTx1);
		QTimer *dpdModelErrorTx1Timer = new QTimer(widget);
		connect(dpdModelErrorTx1Timer, &QTimer::timeout, dpdModelErrorTx1, &IIOWidget::readAsync);
		dpdModelErrorTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, dpdModelErrorTx1, &IIOWidget::readAsync);

		// dpd_external_path_delay TX1 (÷16)
		IIOWidget *dpdPathDelayTx1 = IIOWidgetBuilder(widget)
						     .channel(ch0)
						     .attribute("dpd_external_path_delay")
						     .title("DPD External Path Delay (TX1)")
						     .group(m_group)
						     .buildSingle();
		dpdPathDelayTx1->setEnabled(false);
		dpdPathDelayTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 16.0, 'f', 2); });
		layout->addWidget(dpdPathDelayTx1);
		QTimer *dpdPathDelayTx1Timer = new QTimer(widget);
		connect(dpdPathDelayTx1Timer, &QTimer::timeout, dpdPathDelayTx1, &IIOWidget::readAsync);
		dpdPathDelayTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, dpdPathDelayTx1, &IIOWidget::readAsync);

		// dpd_status TX1 (enum-to-string)
		IIOWidget *dpdStatusTx1 = IIOWidgetBuilder(widget)
						  .channel(ch0)
						  .attribute("dpd_status")
						  .title("DPD Status (TX1)")
						  .group(m_group)
						  .buildSingle();
		dpdStatusTx1->setEnabled(false);
		dpdStatusTx1->setDataToUIConversion([](QString data) {
			int idx = data.toInt();
			if(idx >= 0 && idx < dpdStatusStrings.size())
				return dpdStatusStrings.at(idx);
			return QString("Unknown (%1)").arg(idx);
		});
		layout->addWidget(dpdStatusTx1);
		QTimer *dpdStatusTx1Timer = new QTimer(widget);
		connect(dpdStatusTx1Timer, &QTimer::timeout, dpdStatusTx1, &IIOWidget::readAsync);
		dpdStatusTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, dpdStatusTx1, &IIOWidget::readAsync);

		// DPD TX2 labels (conditional on ch1)
		if(ch1) {
			// dpd_track_count TX2
			IIOWidget *dpdTrackCountTx2 = IIOWidgetBuilder(widget)
							      .channel(ch1)
							      .attribute("dpd_track_count")
							      .title("DPD Track Count (TX2)")
							      .group(m_group)
							      .buildSingle();
			dpdTrackCountTx2->setEnabled(false);
			layout->addWidget(dpdTrackCountTx2);
			QTimer *dpdTrackCountTx2Timer = new QTimer(widget);
			connect(dpdTrackCountTx2Timer, &QTimer::timeout, dpdTrackCountTx2, &IIOWidget::readAsync);
			dpdTrackCountTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, dpdTrackCountTx2, &IIOWidget::readAsync);

			// dpd_model_error TX2 (÷10, %)
			IIOWidget *dpdModelErrorTx2 = IIOWidgetBuilder(widget)
							      .channel(ch1)
							      .attribute("dpd_model_error")
							      .title("DPD Model Error (TX2) (%)")
							      .group(m_group)
							      .buildSingle();
			dpdModelErrorTx2->setEnabled(false);
			dpdModelErrorTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 10.0, 'f', 1); });
			layout->addWidget(dpdModelErrorTx2);
			QTimer *dpdModelErrorTx2Timer = new QTimer(widget);
			connect(dpdModelErrorTx2Timer, &QTimer::timeout, dpdModelErrorTx2, &IIOWidget::readAsync);
			dpdModelErrorTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, dpdModelErrorTx2, &IIOWidget::readAsync);

			// dpd_external_path_delay TX2 (÷16)
			IIOWidget *dpdPathDelayTx2 = IIOWidgetBuilder(widget)
							     .channel(ch1)
							     .attribute("dpd_external_path_delay")
							     .title("DPD External Path Delay (TX2)")
							     .group(m_group)
							     .buildSingle();
			dpdPathDelayTx2->setEnabled(false);
			dpdPathDelayTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 16.0, 'f', 2); });
			layout->addWidget(dpdPathDelayTx2);
			QTimer *dpdPathDelayTx2Timer = new QTimer(widget);
			connect(dpdPathDelayTx2Timer, &QTimer::timeout, dpdPathDelayTx2, &IIOWidget::readAsync);
			dpdPathDelayTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, dpdPathDelayTx2, &IIOWidget::readAsync);

			// dpd_status TX2 (enum-to-string)
			IIOWidget *dpdStatusTx2 = IIOWidgetBuilder(widget)
							  .channel(ch1)
							  .attribute("dpd_status")
							  .title("DPD Status (TX2)")
							  .group(m_group)
							  .buildSingle();
			dpdStatusTx2->setEnabled(false);
			dpdStatusTx2->setDataToUIConversion([](QString data) {
				int idx = data.toInt();
				if(idx >= 0 && idx < dpdStatusStrings.size())
					return dpdStatusStrings.at(idx);
				return QString("Unknown (%1)").arg(idx);
			});
			layout->addWidget(dpdStatusTx2);
			QTimer *dpdStatusTx2Timer = new QTimer(widget);
			connect(dpdStatusTx2Timer, &QTimer::timeout, dpdStatusTx2, &IIOWidget::readAsync);
			dpdStatusTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, dpdStatusTx2, &IIOWidget::readAsync);
		}

		// --- CLGC Status Labels (Template D/F/G, timer 1000ms) ---

		// clgc_track_count TX1
		IIOWidget *clgcTrackCountTx1 = IIOWidgetBuilder(widget)
						       .channel(ch0)
						       .attribute("clgc_track_count")
						       .title("CLGC Track Count (TX1)")
						       .group(m_group)
						       .buildSingle();
		clgcTrackCountTx1->setEnabled(false);
		layout->addWidget(clgcTrackCountTx1);
		QTimer *clgcTrackCountTx1Timer = new QTimer(widget);
		connect(clgcTrackCountTx1Timer, &QTimer::timeout, clgcTrackCountTx1, &IIOWidget::readAsync);
		clgcTrackCountTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, clgcTrackCountTx1, &IIOWidget::readAsync);

		// clgc_status TX1 (enum-to-string)
		IIOWidget *clgcStatusTx1 = IIOWidgetBuilder(widget)
						   .channel(ch0)
						   .attribute("clgc_status")
						   .title("CLGC Status (TX1)")
						   .group(m_group)
						   .buildSingle();
		clgcStatusTx1->setEnabled(false);
		clgcStatusTx1->setDataToUIConversion([](QString data) {
			int idx = data.toInt();
			if(idx >= 0 && idx < clgcStatusStrings.size())
				return clgcStatusStrings.at(idx);
			return QString("Unknown (%1)").arg(idx);
		});
		layout->addWidget(clgcStatusTx1);
		QTimer *clgcStatusTx1Timer = new QTimer(widget);
		connect(clgcStatusTx1Timer, &QTimer::timeout, clgcStatusTx1, &IIOWidget::readAsync);
		clgcStatusTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, clgcStatusTx1, &IIOWidget::readAsync);

		// clgc_current_gain TX1 (÷100, dB)
		IIOWidget *clgcCurrentGainTx1 = IIOWidgetBuilder(widget)
							.channel(ch0)
							.attribute("clgc_current_gain")
							.title("CLGC Current Gain (TX1) (dB)")
							.group(m_group)
							.buildSingle();
		clgcCurrentGainTx1->setEnabled(false);
		clgcCurrentGainTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		layout->addWidget(clgcCurrentGainTx1);
		QTimer *clgcCurrentGainTx1Timer = new QTimer(widget);
		connect(clgcCurrentGainTx1Timer, &QTimer::timeout, clgcCurrentGainTx1, &IIOWidget::readAsync);
		clgcCurrentGainTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, clgcCurrentGainTx1, &IIOWidget::readAsync);

		// clgc_orx_rms TX1 (÷100, dBFS)
		IIOWidget *clgcOrxRmsTx1 = IIOWidgetBuilder(widget)
						   .channel(ch0)
						   .attribute("clgc_orx_rms")
						   .title("CLGC ORx RMS (TX1) (dBFS)")
						   .group(m_group)
						   .buildSingle();
		clgcOrxRmsTx1->setEnabled(false);
		clgcOrxRmsTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		layout->addWidget(clgcOrxRmsTx1);
		QTimer *clgcOrxRmsTx1Timer = new QTimer(widget);
		connect(clgcOrxRmsTx1Timer, &QTimer::timeout, clgcOrxRmsTx1, &IIOWidget::readAsync);
		clgcOrxRmsTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, clgcOrxRmsTx1, &IIOWidget::readAsync);

		// clgc_tx_gain TX1 (÷20, dB)
		IIOWidget *clgcTxGainTx1 = IIOWidgetBuilder(widget)
						   .channel(ch0)
						   .attribute("clgc_tx_gain")
						   .title("CLGC TX Gain (TX1) (dB)")
						   .group(m_group)
						   .buildSingle();
		clgcTxGainTx1->setEnabled(false);
		clgcTxGainTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 20.0, 'f', 2); });
		layout->addWidget(clgcTxGainTx1);
		QTimer *clgcTxGainTx1Timer = new QTimer(widget);
		connect(clgcTxGainTx1Timer, &QTimer::timeout, clgcTxGainTx1, &IIOWidget::readAsync);
		clgcTxGainTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, clgcTxGainTx1, &IIOWidget::readAsync);

		// clgc_tx_rms TX1 (÷100, dBFS)
		IIOWidget *clgcTxRmsTx1 = IIOWidgetBuilder(widget)
						  .channel(ch0)
						  .attribute("clgc_tx_rms")
						  .title("CLGC TX RMS (TX1) (dBFS)")
						  .group(m_group)
						  .buildSingle();
		clgcTxRmsTx1->setEnabled(false);
		clgcTxRmsTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		layout->addWidget(clgcTxRmsTx1);
		QTimer *clgcTxRmsTx1Timer = new QTimer(widget);
		connect(clgcTxRmsTx1Timer, &QTimer::timeout, clgcTxRmsTx1, &IIOWidget::readAsync);
		clgcTxRmsTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, clgcTxRmsTx1, &IIOWidget::readAsync);

		// CLGC TX2 labels (conditional on ch1)
		if(ch1) {
			// clgc_track_count TX2
			IIOWidget *clgcTrackCountTx2 = IIOWidgetBuilder(widget)
							       .channel(ch1)
							       .attribute("clgc_track_count")
							       .title("CLGC Track Count (TX2)")
							       .group(m_group)
							       .buildSingle();
			clgcTrackCountTx2->setEnabled(false);
			layout->addWidget(clgcTrackCountTx2);
			QTimer *clgcTrackCountTx2Timer = new QTimer(widget);
			connect(clgcTrackCountTx2Timer, &QTimer::timeout, clgcTrackCountTx2, &IIOWidget::readAsync);
			clgcTrackCountTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, clgcTrackCountTx2, &IIOWidget::readAsync);

			// clgc_status TX2 (enum-to-string)
			IIOWidget *clgcStatusTx2 = IIOWidgetBuilder(widget)
							   .channel(ch1)
							   .attribute("clgc_status")
							   .title("CLGC Status (TX2)")
							   .group(m_group)
							   .buildSingle();
			clgcStatusTx2->setEnabled(false);
			clgcStatusTx2->setDataToUIConversion([](QString data) {
				int idx = data.toInt();
				if(idx >= 0 && idx < clgcStatusStrings.size())
					return clgcStatusStrings.at(idx);
				return QString("Unknown (%1)").arg(idx);
			});
			layout->addWidget(clgcStatusTx2);
			QTimer *clgcStatusTx2Timer = new QTimer(widget);
			connect(clgcStatusTx2Timer, &QTimer::timeout, clgcStatusTx2, &IIOWidget::readAsync);
			clgcStatusTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, clgcStatusTx2, &IIOWidget::readAsync);

			// clgc_current_gain TX2 (÷100, dB)
			IIOWidget *clgcCurrentGainTx2 = IIOWidgetBuilder(widget)
								.channel(ch1)
								.attribute("clgc_current_gain")
								.title("CLGC Current Gain (TX2) (dB)")
								.group(m_group)
								.buildSingle();
			clgcCurrentGainTx2->setEnabled(false);
			clgcCurrentGainTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			layout->addWidget(clgcCurrentGainTx2);
			QTimer *clgcCurrentGainTx2Timer = new QTimer(widget);
			connect(clgcCurrentGainTx2Timer, &QTimer::timeout, clgcCurrentGainTx2, &IIOWidget::readAsync);
			clgcCurrentGainTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, clgcCurrentGainTx2, &IIOWidget::readAsync);

			// clgc_orx_rms TX2 (÷100, dBFS)
			IIOWidget *clgcOrxRmsTx2 = IIOWidgetBuilder(widget)
							   .channel(ch1)
							   .attribute("clgc_orx_rms")
							   .title("CLGC ORx RMS (TX2) (dBFS)")
							   .group(m_group)
							   .buildSingle();
			clgcOrxRmsTx2->setEnabled(false);
			clgcOrxRmsTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			layout->addWidget(clgcOrxRmsTx2);
			QTimer *clgcOrxRmsTx2Timer = new QTimer(widget);
			connect(clgcOrxRmsTx2Timer, &QTimer::timeout, clgcOrxRmsTx2, &IIOWidget::readAsync);
			clgcOrxRmsTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, clgcOrxRmsTx2, &IIOWidget::readAsync);

			// clgc_tx_gain TX2 (÷20, dB)
			IIOWidget *clgcTxGainTx2 = IIOWidgetBuilder(widget)
							   .channel(ch1)
							   .attribute("clgc_tx_gain")
							   .title("CLGC TX Gain (TX2) (dB)")
							   .group(m_group)
							   .buildSingle();
			clgcTxGainTx2->setEnabled(false);
			clgcTxGainTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 20.0, 'f', 2); });
			layout->addWidget(clgcTxGainTx2);
			QTimer *clgcTxGainTx2Timer = new QTimer(widget);
			connect(clgcTxGainTx2Timer, &QTimer::timeout, clgcTxGainTx2, &IIOWidget::readAsync);
			clgcTxGainTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, clgcTxGainTx2, &IIOWidget::readAsync);

			// clgc_tx_rms TX2 (÷100, dBFS)
			IIOWidget *clgcTxRmsTx2 = IIOWidgetBuilder(widget)
							  .channel(ch1)
							  .attribute("clgc_tx_rms")
							  .title("CLGC TX RMS (TX2) (dBFS)")
							  .group(m_group)
							  .buildSingle();
			clgcTxRmsTx2->setEnabled(false);
			clgcTxRmsTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			layout->addWidget(clgcTxRmsTx2);
			QTimer *clgcTxRmsTx2Timer = new QTimer(widget);
			connect(clgcTxRmsTx2Timer, &QTimer::timeout, clgcTxRmsTx2, &IIOWidget::readAsync);
			clgcTxRmsTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, clgcTxRmsTx2, &IIOWidget::readAsync);
		}

		// --- VSWR Status Labels (Template D/F/G/H, timer 1000ms) ---

		// vswr_track_count TX1
		IIOWidget *vswrTrackCountTx1 = IIOWidgetBuilder(widget)
						       .channel(ch0)
						       .attribute("vswr_track_count")
						       .title("VSWR Track Count (TX1)")
						       .group(m_group)
						       .buildSingle();
		vswrTrackCountTx1->setEnabled(false);
		layout->addWidget(vswrTrackCountTx1);
		QTimer *vswrTrackCountTx1Timer = new QTimer(widget);
		connect(vswrTrackCountTx1Timer, &QTimer::timeout, vswrTrackCountTx1, &IIOWidget::readAsync);
		vswrTrackCountTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrTrackCountTx1, &IIOWidget::readAsync);

		// vswr_status TX1 (enum-to-string)
		IIOWidget *vswrStatusTx1 = IIOWidgetBuilder(widget)
						   .channel(ch0)
						   .attribute("vswr_status")
						   .title("VSWR Status (TX1)")
						   .group(m_group)
						   .buildSingle();
		vswrStatusTx1->setEnabled(false);
		vswrStatusTx1->setDataToUIConversion([](QString data) {
			int idx = data.toInt();
			if(idx >= 0 && idx < vswrStatusStrings.size())
				return vswrStatusStrings.at(idx);
			return QString("Unknown (%1)").arg(idx);
		});
		layout->addWidget(vswrStatusTx1);
		QTimer *vswrStatusTx1Timer = new QTimer(widget);
		connect(vswrStatusTx1Timer, &QTimer::timeout, vswrStatusTx1, &IIOWidget::readAsync);
		vswrStatusTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrStatusTx1, &IIOWidget::readAsync);

		// vswr_forward_gain TX1 (÷100, dB)
		IIOWidget *vswrFwdGainTx1 = IIOWidgetBuilder(widget)
						    .channel(ch0)
						    .attribute("vswr_forward_gain")
						    .title("VSWR Forward Gain (TX1) (dB)")
						    .group(m_group)
						    .buildSingle();
		vswrFwdGainTx1->setEnabled(false);
		vswrFwdGainTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		layout->addWidget(vswrFwdGainTx1);
		QTimer *vswrFwdGainTx1Timer = new QTimer(widget);
		connect(vswrFwdGainTx1Timer, &QTimer::timeout, vswrFwdGainTx1, &IIOWidget::readAsync);
		vswrFwdGainTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrFwdGainTx1, &IIOWidget::readAsync);

		// vswr_forward_gain_imag TX1 (÷100, dB)
		IIOWidget *vswrFwdGainImagTx1 = IIOWidgetBuilder(widget)
							.channel(ch0)
							.attribute("vswr_forward_gain_imag")
							.title("VSWR Forward Gain Imag (TX1) (dB)")
							.group(m_group)
							.buildSingle();
		vswrFwdGainImagTx1->setEnabled(false);
		vswrFwdGainImagTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		layout->addWidget(vswrFwdGainImagTx1);
		QTimer *vswrFwdGainImagTx1Timer = new QTimer(widget);
		connect(vswrFwdGainImagTx1Timer, &QTimer::timeout, vswrFwdGainImagTx1, &IIOWidget::readAsync);
		vswrFwdGainImagTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrFwdGainImagTx1, &IIOWidget::readAsync);

		// vswr_forward_gain_real TX1 (÷100, dB)
		IIOWidget *vswrFwdGainRealTx1 = IIOWidgetBuilder(widget)
							.channel(ch0)
							.attribute("vswr_forward_gain_real")
							.title("VSWR Forward Gain Real (TX1) (dB)")
							.group(m_group)
							.buildSingle();
		vswrFwdGainRealTx1->setEnabled(false);
		vswrFwdGainRealTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		layout->addWidget(vswrFwdGainRealTx1);
		QTimer *vswrFwdGainRealTx1Timer = new QTimer(widget);
		connect(vswrFwdGainRealTx1Timer, &QTimer::timeout, vswrFwdGainRealTx1, &IIOWidget::readAsync);
		vswrFwdGainRealTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrFwdGainRealTx1, &IIOWidget::readAsync);

		// vswr_forward_orx TX1 (PRMS: ÷100+21, dBFS)
		IIOWidget *vswrFwdOrxTx1 = IIOWidgetBuilder(widget)
						   .channel(ch0)
						   .attribute("vswr_forward_orx")
						   .title("VSWR Forward ORx (TX1) (dBFS)")
						   .group(m_group)
						   .buildSingle();
		vswrFwdOrxTx1->setEnabled(false);
		vswrFwdOrxTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0 + 21.0, 'f', 2); });
		layout->addWidget(vswrFwdOrxTx1);
		QTimer *vswrFwdOrxTx1Timer = new QTimer(widget);
		connect(vswrFwdOrxTx1Timer, &QTimer::timeout, vswrFwdOrxTx1, &IIOWidget::readAsync);
		vswrFwdOrxTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrFwdOrxTx1, &IIOWidget::readAsync);

		// vswr_forward_tx TX1 (PRMS: ÷100+21, dBFS)
		IIOWidget *vswrFwdTxTx1 = IIOWidgetBuilder(widget)
						  .channel(ch0)
						  .attribute("vswr_forward_tx")
						  .title("VSWR Forward TX (TX1) (dBFS)")
						  .group(m_group)
						  .buildSingle();
		vswrFwdTxTx1->setEnabled(false);
		vswrFwdTxTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0 + 21.0, 'f', 2); });
		layout->addWidget(vswrFwdTxTx1);
		QTimer *vswrFwdTxTx1Timer = new QTimer(widget);
		connect(vswrFwdTxTx1Timer, &QTimer::timeout, vswrFwdTxTx1, &IIOWidget::readAsync);
		vswrFwdTxTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrFwdTxTx1, &IIOWidget::readAsync);

		// vswr_reflected_gain TX1 (÷100, dB)
		IIOWidget *vswrRefGainTx1 = IIOWidgetBuilder(widget)
						    .channel(ch0)
						    .attribute("vswr_reflected_gain")
						    .title("VSWR Reflected Gain (TX1) (dB)")
						    .group(m_group)
						    .buildSingle();
		vswrRefGainTx1->setEnabled(false);
		vswrRefGainTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		layout->addWidget(vswrRefGainTx1);
		QTimer *vswrRefGainTx1Timer = new QTimer(widget);
		connect(vswrRefGainTx1Timer, &QTimer::timeout, vswrRefGainTx1, &IIOWidget::readAsync);
		vswrRefGainTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrRefGainTx1, &IIOWidget::readAsync);

		// vswr_reflected_gain_imag TX1 (÷100, dB)
		IIOWidget *vswrRefGainImagTx1 = IIOWidgetBuilder(widget)
							.channel(ch0)
							.attribute("vswr_reflected_gain_imag")
							.title("VSWR Reflected Gain Imag (TX1) (dB)")
							.group(m_group)
							.buildSingle();
		vswrRefGainImagTx1->setEnabled(false);
		vswrRefGainImagTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		layout->addWidget(vswrRefGainImagTx1);
		QTimer *vswrRefGainImagTx1Timer = new QTimer(widget);
		connect(vswrRefGainImagTx1Timer, &QTimer::timeout, vswrRefGainImagTx1, &IIOWidget::readAsync);
		vswrRefGainImagTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrRefGainImagTx1, &IIOWidget::readAsync);

		// vswr_reflected_gain_real TX1 (÷100, dB)
		IIOWidget *vswrRefGainRealTx1 = IIOWidgetBuilder(widget)
							.channel(ch0)
							.attribute("vswr_reflected_gain_real")
							.title("VSWR Reflected Gain Real (TX1) (dB)")
							.group(m_group)
							.buildSingle();
		vswrRefGainRealTx1->setEnabled(false);
		vswrRefGainRealTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
		layout->addWidget(vswrRefGainRealTx1);
		QTimer *vswrRefGainRealTx1Timer = new QTimer(widget);
		connect(vswrRefGainRealTx1Timer, &QTimer::timeout, vswrRefGainRealTx1, &IIOWidget::readAsync);
		vswrRefGainRealTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrRefGainRealTx1, &IIOWidget::readAsync);

		// vswr_reflected_orx TX1 (PRMS: ÷100+21, dBFS)
		IIOWidget *vswrRefOrxTx1 = IIOWidgetBuilder(widget)
						   .channel(ch0)
						   .attribute("vswr_reflected_orx")
						   .title("VSWR Reflected ORx (TX1) (dBFS)")
						   .group(m_group)
						   .buildSingle();
		vswrRefOrxTx1->setEnabled(false);
		vswrRefOrxTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0 + 21.0, 'f', 2); });
		layout->addWidget(vswrRefOrxTx1);
		QTimer *vswrRefOrxTx1Timer = new QTimer(widget);
		connect(vswrRefOrxTx1Timer, &QTimer::timeout, vswrRefOrxTx1, &IIOWidget::readAsync);
		vswrRefOrxTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrRefOrxTx1, &IIOWidget::readAsync);

		// vswr_reflected_tx TX1 (PRMS: ÷100+21, dBFS)
		IIOWidget *vswrRefTxTx1 = IIOWidgetBuilder(widget)
						  .channel(ch0)
						  .attribute("vswr_reflected_tx")
						  .title("VSWR Reflected TX (TX1) (dBFS)")
						  .group(m_group)
						  .buildSingle();
		vswrRefTxTx1->setEnabled(false);
		vswrRefTxTx1->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 100.0 + 21.0, 'f', 2); });
		layout->addWidget(vswrRefTxTx1);
		QTimer *vswrRefTxTx1Timer = new QTimer(widget);
		connect(vswrRefTxTx1Timer, &QTimer::timeout, vswrRefTxTx1, &IIOWidget::readAsync);
		vswrRefTxTx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, vswrRefTxTx1, &IIOWidget::readAsync);

		// VSWR TX2 labels (conditional on ch1)
		if(ch1) {
			// vswr_track_count TX2
			IIOWidget *vswrTrackCountTx2 = IIOWidgetBuilder(widget)
							       .channel(ch1)
							       .attribute("vswr_track_count")
							       .title("VSWR Track Count (TX2)")
							       .group(m_group)
							       .buildSingle();
			vswrTrackCountTx2->setEnabled(false);
			layout->addWidget(vswrTrackCountTx2);
			QTimer *vswrTrackCountTx2Timer = new QTimer(widget);
			connect(vswrTrackCountTx2Timer, &QTimer::timeout, vswrTrackCountTx2, &IIOWidget::readAsync);
			vswrTrackCountTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrTrackCountTx2, &IIOWidget::readAsync);

			// vswr_status TX2 (enum-to-string)
			IIOWidget *vswrStatusTx2 = IIOWidgetBuilder(widget)
							   .channel(ch1)
							   .attribute("vswr_status")
							   .title("VSWR Status (TX2)")
							   .group(m_group)
							   .buildSingle();
			vswrStatusTx2->setEnabled(false);
			vswrStatusTx2->setDataToUIConversion([](QString data) {
				int idx = data.toInt();
				if(idx >= 0 && idx < vswrStatusStrings.size())
					return vswrStatusStrings.at(idx);
				return QString("Unknown (%1)").arg(idx);
			});
			layout->addWidget(vswrStatusTx2);
			QTimer *vswrStatusTx2Timer = new QTimer(widget);
			connect(vswrStatusTx2Timer, &QTimer::timeout, vswrStatusTx2, &IIOWidget::readAsync);
			vswrStatusTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrStatusTx2, &IIOWidget::readAsync);

			// vswr_forward_gain TX2 (÷100, dB)
			IIOWidget *vswrFwdGainTx2 = IIOWidgetBuilder(widget)
							    .channel(ch1)
							    .attribute("vswr_forward_gain")
							    .title("VSWR Forward Gain (TX2) (dB)")
							    .group(m_group)
							    .buildSingle();
			vswrFwdGainTx2->setEnabled(false);
			vswrFwdGainTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			layout->addWidget(vswrFwdGainTx2);
			QTimer *vswrFwdGainTx2Timer = new QTimer(widget);
			connect(vswrFwdGainTx2Timer, &QTimer::timeout, vswrFwdGainTx2, &IIOWidget::readAsync);
			vswrFwdGainTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrFwdGainTx2, &IIOWidget::readAsync);

			// vswr_forward_gain_imag TX2 (÷100, dB)
			IIOWidget *vswrFwdGainImagTx2 = IIOWidgetBuilder(widget)
								.channel(ch1)
								.attribute("vswr_forward_gain_imag")
								.title("VSWR Forward Gain Imag (TX2) (dB)")
								.group(m_group)
								.buildSingle();
			vswrFwdGainImagTx2->setEnabled(false);
			vswrFwdGainImagTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			layout->addWidget(vswrFwdGainImagTx2);
			QTimer *vswrFwdGainImagTx2Timer = new QTimer(widget);
			connect(vswrFwdGainImagTx2Timer, &QTimer::timeout, vswrFwdGainImagTx2, &IIOWidget::readAsync);
			vswrFwdGainImagTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrFwdGainImagTx2, &IIOWidget::readAsync);

			// vswr_forward_gain_real TX2 (÷100, dB)
			IIOWidget *vswrFwdGainRealTx2 = IIOWidgetBuilder(widget)
								.channel(ch1)
								.attribute("vswr_forward_gain_real")
								.title("VSWR Forward Gain Real (TX2) (dB)")
								.group(m_group)
								.buildSingle();
			vswrFwdGainRealTx2->setEnabled(false);
			vswrFwdGainRealTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			layout->addWidget(vswrFwdGainRealTx2);
			QTimer *vswrFwdGainRealTx2Timer = new QTimer(widget);
			connect(vswrFwdGainRealTx2Timer, &QTimer::timeout, vswrFwdGainRealTx2, &IIOWidget::readAsync);
			vswrFwdGainRealTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrFwdGainRealTx2, &IIOWidget::readAsync);

			// vswr_forward_orx TX2 (PRMS: ÷100+21, dBFS)
			IIOWidget *vswrFwdOrxTx2 = IIOWidgetBuilder(widget)
							   .channel(ch1)
							   .attribute("vswr_forward_orx")
							   .title("VSWR Forward ORx (TX2) (dBFS)")
							   .group(m_group)
							   .buildSingle();
			vswrFwdOrxTx2->setEnabled(false);
			vswrFwdOrxTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0 + 21.0, 'f', 2); });
			layout->addWidget(vswrFwdOrxTx2);
			QTimer *vswrFwdOrxTx2Timer = new QTimer(widget);
			connect(vswrFwdOrxTx2Timer, &QTimer::timeout, vswrFwdOrxTx2, &IIOWidget::readAsync);
			vswrFwdOrxTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrFwdOrxTx2, &IIOWidget::readAsync);

			// vswr_forward_tx TX2 (PRMS: ÷100+21, dBFS)
			IIOWidget *vswrFwdTxTx2 = IIOWidgetBuilder(widget)
							  .channel(ch1)
							  .attribute("vswr_forward_tx")
							  .title("VSWR Forward TX (TX2) (dBFS)")
							  .group(m_group)
							  .buildSingle();
			vswrFwdTxTx2->setEnabled(false);
			vswrFwdTxTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0 + 21.0, 'f', 2); });
			layout->addWidget(vswrFwdTxTx2);
			QTimer *vswrFwdTxTx2Timer = new QTimer(widget);
			connect(vswrFwdTxTx2Timer, &QTimer::timeout, vswrFwdTxTx2, &IIOWidget::readAsync);
			vswrFwdTxTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrFwdTxTx2, &IIOWidget::readAsync);

			// vswr_reflected_gain TX2 (÷100, dB)
			IIOWidget *vswrRefGainTx2 = IIOWidgetBuilder(widget)
							    .channel(ch1)
							    .attribute("vswr_reflected_gain")
							    .title("VSWR Reflected Gain (TX2) (dB)")
							    .group(m_group)
							    .buildSingle();
			vswrRefGainTx2->setEnabled(false);
			vswrRefGainTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			layout->addWidget(vswrRefGainTx2);
			QTimer *vswrRefGainTx2Timer = new QTimer(widget);
			connect(vswrRefGainTx2Timer, &QTimer::timeout, vswrRefGainTx2, &IIOWidget::readAsync);
			vswrRefGainTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrRefGainTx2, &IIOWidget::readAsync);

			// vswr_reflected_gain_imag TX2 (÷100, dB)
			IIOWidget *vswrRefGainImagTx2 = IIOWidgetBuilder(widget)
								.channel(ch1)
								.attribute("vswr_reflected_gain_imag")
								.title("VSWR Reflected Gain Imag (TX2) (dB)")
								.group(m_group)
								.buildSingle();
			vswrRefGainImagTx2->setEnabled(false);
			vswrRefGainImagTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			layout->addWidget(vswrRefGainImagTx2);
			QTimer *vswrRefGainImagTx2Timer = new QTimer(widget);
			connect(vswrRefGainImagTx2Timer, &QTimer::timeout, vswrRefGainImagTx2, &IIOWidget::readAsync);
			vswrRefGainImagTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrRefGainImagTx2, &IIOWidget::readAsync);

			// vswr_reflected_gain_real TX2 (÷100, dB)
			IIOWidget *vswrRefGainRealTx2 = IIOWidgetBuilder(widget)
								.channel(ch1)
								.attribute("vswr_reflected_gain_real")
								.title("VSWR Reflected Gain Real (TX2) (dB)")
								.group(m_group)
								.buildSingle();
			vswrRefGainRealTx2->setEnabled(false);
			vswrRefGainRealTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0, 'f', 2); });
			layout->addWidget(vswrRefGainRealTx2);
			QTimer *vswrRefGainRealTx2Timer = new QTimer(widget);
			connect(vswrRefGainRealTx2Timer, &QTimer::timeout, vswrRefGainRealTx2, &IIOWidget::readAsync);
			vswrRefGainRealTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrRefGainRealTx2, &IIOWidget::readAsync);

			// vswr_reflected_orx TX2 (PRMS: ÷100+21, dBFS)
			IIOWidget *vswrRefOrxTx2 = IIOWidgetBuilder(widget)
							   .channel(ch1)
							   .attribute("vswr_reflected_orx")
							   .title("VSWR Reflected ORx (TX2) (dBFS)")
							   .group(m_group)
							   .buildSingle();
			vswrRefOrxTx2->setEnabled(false);
			vswrRefOrxTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0 + 21.0, 'f', 2); });
			layout->addWidget(vswrRefOrxTx2);
			QTimer *vswrRefOrxTx2Timer = new QTimer(widget);
			connect(vswrRefOrxTx2Timer, &QTimer::timeout, vswrRefOrxTx2, &IIOWidget::readAsync);
			vswrRefOrxTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrRefOrxTx2, &IIOWidget::readAsync);

			// vswr_reflected_tx TX2 (PRMS: ÷100+21, dBFS)
			IIOWidget *vswrRefTxTx2 = IIOWidgetBuilder(widget)
							  .channel(ch1)
							  .attribute("vswr_reflected_tx")
							  .title("VSWR Reflected TX (TX2) (dBFS)")
							  .group(m_group)
							  .buildSingle();
			vswrRefTxTx2->setEnabled(false);
			vswrRefTxTx2->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 100.0 + 21.0, 'f', 2); });
			layout->addWidget(vswrRefTxTx2);
			QTimer *vswrRefTxTx2Timer = new QTimer(widget);
			connect(vswrRefTxTx2Timer, &QTimer::timeout, vswrRefTxTx2, &IIOWidget::readAsync);
			vswrRefTxTx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, vswrRefTxTx2, &IIOWidget::readAsync);
		}
	}

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}

QWidget *AD9371Helper::generateFpgaWidget(iio_device *dds, iio_device *cap, bool is_2rx_2tx, QString title,
					  QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	// FPGA TX sampling_frequency (conditional on _available attr)
	if(dds) {
		iio_channel *ddsCh0 = iio_device_find_channel(dds, "voltage0", true);
		if(ddsCh0 && iio_channel_find_attr(ddsCh0, "sampling_frequency_available")) {
			IIOWidget *fpgaTxFreq = IIOWidgetBuilder(widget)
							.channel(ddsCh0)
							.attribute("sampling_frequency")
							.optionsAttribute("sampling_frequency_available")
							.title("FPGA TX Sampling Frequency")
							.uiStrategy(IIOWidgetBuilder::ComboUi)
							.group(m_group)
							.buildSingle();
			layout->addWidget(fpgaTxFreq);
			connect(this, &AD9371Helper::readRequested, fpgaTxFreq, &IIOWidget::readAsync);
		}
	}

	// FPGA RX sampling_frequency (conditional on _available attr)
	if(cap) {
		iio_channel *capCh0 = iio_device_find_channel(cap, "voltage0_i", false);
		if(capCh0 && iio_channel_find_attr(capCh0, "sampling_frequency_available")) {
			IIOWidget *fpgaRxFreq = IIOWidgetBuilder(widget)
							.channel(capCh0)
							.attribute("sampling_frequency")
							.optionsAttribute("sampling_frequency_available")
							.title("FPGA RX Sampling Frequency")
							.uiStrategy(IIOWidgetBuilder::ComboUi)
							.group(m_group)
							.buildSingle();
			layout->addWidget(fpgaRxFreq);
			connect(this, &AD9371Helper::readRequested, fpgaRxFreq, &IIOWidget::readAsync);
		}
	}

	// RX Phase Rotation — computed from calibscale/calibphase on capture device
	// Uses trig math from ad9371.c:844-904 (read) and 1018-1045 (write)
	if(cap) {
		// Helper lambda: read calibscale/calibphase from I/Q channels, convert to degrees
		auto readPhaseRotation = [](iio_device *capDev, const char *chI, const char *chQ) -> double {
			iio_channel *out0 = iio_device_find_channel(capDev, chI, false);
			iio_channel *out1 = iio_device_find_channel(capDev, chQ, false);
			if(!out0 || !out1)
				return 0.0;
			double val[4];
			iio_channel_attr_read_double(out0, "calibscale", &val[0]);
			iio_channel_attr_read_double(out0, "calibphase", &val[1]);
			iio_channel_attr_read_double(out1, "calibscale", &val[2]);
			iio_channel_attr_read_double(out1, "calibphase", &val[3]);

			val[0] = acos(val[0]) * 360.0 / (2.0 * M_PI);
			val[1] = asin(-1.0 * val[1]) * 360.0 / (2.0 * M_PI);
			val[2] = acos(val[2]) * 360.0 / (2.0 * M_PI);
			val[3] = asin(val[3]) * 360.0 / (2.0 * M_PI);

			if(val[1] < 0.0)
				val[0] *= -1.0;
			if(val[3] < 0.0)
				val[2] *= -1.0;
			if(val[1] < -90.0)
				val[0] = (val[0] * -1.0) - 180.0;
			if(val[3] < -90.0)
				val[2] = (val[2] * -1.0) - 180.0;

			if(fabs(val[0]) > 90.0) {
				if(val[1] < 0.0)
					val[1] = (val[1] * -1.0) - 180.0;
				else
					val[1] = 180.0 - val[1];
			}
			if(fabs(val[2]) > 90.0) {
				if(val[3] < 0.0)
					val[3] = (val[3] * -1.0) - 180.0;
				else
					val[3] = 180.0 - val[3];
			}

			if(round(val[0]) != round(val[1]) && round(val[0]) != round(val[2]) &&
			   round(val[0]) != round(val[3])) {
				return 0.0;
			}
			return (val[0] + val[1] + val[2] + val[3]) / 4.0;
		};

		// Helper lambda: write phase angle (degrees) to I/Q channels as calibscale/calibphase
		auto writePhaseRotation = [](iio_device *capDev, const char *chI, const char *chQ, double degrees) {
			iio_channel *out0 = iio_device_find_channel(capDev, chI, false);
			iio_channel *out1 = iio_device_find_channel(capDev, chQ, false);
			if(!out0 || !out1)
				return;
			double phase = degrees * 2.0 * M_PI / 360.0;
			iio_channel_attr_write_double(out0, "calibscale", cos(phase));
			iio_channel_attr_write_double(out0, "calibphase", -sin(phase));
			iio_channel_attr_write_double(out1, "calibscale", cos(phase));
			iio_channel_attr_write_double(out1, "calibphase", sin(phase));
		};

		// Side-by-side phase rotation
		QHBoxLayout *phaseRow = new QHBoxLayout();

		// RX1 Phase Rotation
		QDoubleSpinBox *phaseRx1 = new QDoubleSpinBox(widget);
		phaseRx1->setRange(-180.0, 180.0);
		phaseRx1->setSingleStep(1.0);
		phaseRx1->setDecimals(2);
		phaseRx1->setPrefix("RX1 Phase Rotation: ");
		phaseRx1->setSuffix(" degrees");
		phaseRow->addWidget(phaseRx1);

		connect(phaseRx1, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
			[cap, writePhaseRotation](double val) {
				writePhaseRotation(cap, "voltage0_i", "voltage0_q", val);
			});

		auto updatePhaseRx1 = [cap, phaseRx1, readPhaseRotation]() {
			double result = readPhaseRotation(cap, "voltage0_i", "voltage0_q");
			phaseRx1->blockSignals(true);
			phaseRx1->setValue(result);
			phaseRx1->blockSignals(false);
		};
		QTimer *phaseRx1Timer = new QTimer(widget);
		connect(phaseRx1Timer, &QTimer::timeout, widget, updatePhaseRx1);
		phaseRx1Timer->start(1000);
		connect(this, &AD9371Helper::readRequested, widget, updatePhaseRx1);
		updatePhaseRx1(); // initial read

		// RX2 Phase Rotation (conditional on is_2rx_2tx)
		if(is_2rx_2tx) {
			QDoubleSpinBox *phaseRx2 = new QDoubleSpinBox(widget);
			phaseRx2->setRange(-180.0, 180.0);
			phaseRx2->setSingleStep(1.0);
			phaseRx2->setDecimals(2);
			phaseRx2->setPrefix("RX2 Phase Rotation: ");
			phaseRx2->setSuffix(" degrees");
			phaseRow->addWidget(phaseRx2);

			connect(phaseRx2, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
				[cap, writePhaseRotation](double val) {
					writePhaseRotation(cap, "voltage1_i", "voltage1_q", val);
				});

			auto updatePhaseRx2 = [cap, phaseRx2, readPhaseRotation]() {
				double result = readPhaseRotation(cap, "voltage1_i", "voltage1_q");
				phaseRx2->blockSignals(true);
				phaseRx2->setValue(result);
				phaseRx2->blockSignals(false);
			};
			QTimer *phaseRx2Timer = new QTimer(widget);
			connect(phaseRx2Timer, &QTimer::timeout, widget, updatePhaseRx2);
			phaseRx2Timer->start(1000);
			connect(this, &AD9371Helper::readRequested, widget, updatePhaseRx2);
			updatePhaseRx2(); // initial read
		}

		layout->addLayout(phaseRow);
	}

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}
