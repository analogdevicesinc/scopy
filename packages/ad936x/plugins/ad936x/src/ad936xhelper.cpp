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

#include "ad936xhelper.h"

#include "fastlockprofileswidget.h"
#include "firfilterqwidget.h"

#include <menuonoffswitch.h>
#include <style.h>

#include <guistrategy/comboguistrategy.h>

Q_LOGGING_CATEGORY(CAT_AD936X_HELPER, "AD936X_HELPER");

using namespace scopy;
using namespace ad936x;

AD936xHelper::AD936xHelper(QWidget *parent)
	: QWidget(parent)
{}

QWidget *AD936xHelper::generateGlobalSettingsWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *globalSettingsWidget = new QWidget(parent);
	Style::setBackgroundColor(globalSettingsWidget, json::theme::background_primary);
	Style::setStyle(globalSettingsWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(globalSettingsWidget);
	globalSettingsWidget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, globalSettingsWidget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	QHBoxLayout *hlayout = new QHBoxLayout();

	//// ensm_mode
	IIOWidget *ensmMode = IIOWidgetBuilder(globalSettingsWidget)
				      .device(dev)
				      .attribute("ensm_mode")
				      .optionsAttribute("ensm_mode_available")
				      .title("ENSM Mode")
				      .uiStrategy(IIOWidgetBuilder::ComboUi)
				      .buildSingle();

	hlayout->addWidget(ensmMode);

	connect(this, &AD936xHelper::readRequested, ensmMode, &IIOWidget::readAsync);

	////calib_mode
	IIOWidget *calibMode = IIOWidgetBuilder(globalSettingsWidget)
				       .device(dev)
				       .attribute("calib_mode")
				       .optionsAttribute("calib_mode_available")
				       .title("Calibration Mode")
				       .uiStrategy(IIOWidgetBuilder::ComboUi)
				       .buildSingle();
	hlayout->addWidget(calibMode);
	connect(this, &AD936xHelper::readRequested, calibMode, &IIOWidget::readAsync);

	Style::setStyle(calibMode, style::properties::widget::basicBackground, true, true);

	// trx_rate_governor
	IIOWidget *trxRateGovernor = IIOWidgetBuilder(globalSettingsWidget)
					     .device(dev)
					     .attribute("trx_rate_governor")
					     .optionsAttribute("trx_rate_governor_available")
					     .title("TRX Rate Governor")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .buildSingle();
	hlayout->addWidget(trxRateGovernor);
	connect(this, &AD936xHelper::readRequested, trxRateGovernor, &IIOWidget::readAsync);

	FirFilterQWidget *firFilter = new FirFilterQWidget(dev, nullptr, globalSettingsWidget);
	hlayout->addWidget(firFilter);

	layout->addLayout(hlayout);

	// rx_path_rates
	IIOWidget *rxPathRates = IIOWidgetBuilder(globalSettingsWidget)
					 .device(dev)
					 .attribute("rx_path_rates")
					 .title("RX Path Rates")
					 .buildSingle();
	layout->addWidget(rxPathRates);
	rxPathRates->setEnabled(false);
	connect(this, &AD936xHelper::readRequested, rxPathRates, &IIOWidget::readAsync);

	// tx_path_rates
	IIOWidget *txPathRates = IIOWidgetBuilder(globalSettingsWidget)
					 .device(dev)
					 .attribute("tx_path_rates")
					 .title("Tx Path Rates")
					 .buildSingle();
	layout->addWidget(txPathRates);
	txPathRates->setEnabled(false);
	connect(this, &AD936xHelper::readRequested, txPathRates, &IIOWidget::readAsync);

	connect(firFilter, &FirFilterQWidget::filterChanged, this, [=, this]() {
		rxPathRates->read();
		txPathRates->read();
	});

	// xo_correction
	IIOWidget *xoCorrection = IIOWidgetBuilder(globalSettingsWidget)
					  .device(dev)
					  .attribute("xo_correction")
					  .optionsAttribute("xo_correction_available")
					  .title("XO Correction")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .buildSingle();
	layout->addWidget(xoCorrection);
	connect(this, &AD936xHelper::readRequested, xoCorrection, &IIOWidget::readAsync);

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return globalSettingsWidget;
}

QWidget *AD936xHelper::generateRxDeviceWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QGridLayout *layout = new QGridLayout(widget);
	layout->setSpacing(10);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel, 0, 0);

	// because this channel is marked as output by libiio we need to mark altvoltage0 as output
	iio_channel *altVoltage0 = iio_device_find_channel(dev, "altvoltage0", true);

	// altvoltage0: RX_LO // frequency
	IIOWidget *altVoltage0Frequency = IIOWidgetBuilder(widget)
						  .channel(altVoltage0)
						  .attribute("frequency")
						  .optionsAttribute("frequency_available")
						  .title("RX LO Frequency(MHz)")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .buildSingle();
	altVoltage0Frequency->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	altVoltage0Frequency->setRangeToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	altVoltage0Frequency->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });

	connect(this, &AD936xHelper::readRequested, altVoltage0Frequency, &IIOWidget::readAsync);

	MenuOnOffSwitch *useExternalRxLo = new MenuOnOffSwitch("External Rx LO", widget, false);
	useExternalRxLo->onOffswitch()->setChecked(true);

	layout->addWidget(altVoltage0Frequency, 1, 0, 2, 1);
	layout->addWidget(useExternalRxLo, 2, 0, 1, 1);

	// fastlock profile
	FastlockProfilesWidget *fastlockProfile = new FastlockProfilesWidget(altVoltage0, this);

	connect(useExternalRxLo->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { fastlockProfile->setEnabled(toggled); });

	layout->addWidget(fastlockProfile, 1, 1, 2, 1);

	connect(fastlockProfile, &FastlockProfilesWidget::recallCalled, this,
		[=, this] { altVoltage0Frequency->read(); });

	return widget;
}

QWidget *AD936xHelper::generateRxChannelWidget(iio_channel *chn, QString title, QWidget *parent)
{
	QWidget *rxWidget = new QWidget(parent);
	Style::setStyle(rxWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(rxWidget);
	rxWidget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, rxWidget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	// voltage0: hardwaregain
	IIOWidget *hardwaregain = IIOWidgetBuilder(rxWidget)
					  .channel(chn)
					  .attribute("hardwaregain")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .optionsAttribute("hardwaregain_available")
					  .title("Hardware Gain(dB)")
					  .buildSingle();
	layout->addWidget(hardwaregain);
	connect(this, &AD936xHelper::readRequested, hardwaregain, &IIOWidget::readAsync);

	hardwaregain->setDataToUIConversion([this](QString data) {
		// data has dB as string in the value
		auto result = data.split(" ");
		return result.first();
	});

	hardwaregain->lastReturnCode();

	// voltage: rssi
	IIOWidget *rssi = IIOWidgetBuilder(rxWidget).channel(chn).attribute("rssi").title("RSSI(dB)").buildSingle();
	layout->addWidget(rssi);
	rssi->setEnabled(false);

	QTimer *timer = new QTimer(rxWidget);

	QObject::connect(timer, &QTimer::timeout, rssi, &IIOWidget::readAsync);

	timer->start(1000);

	// voltage: gain_control_mode
	IIOWidget *gainControlMode = IIOWidgetBuilder(rxWidget)
					     .channel(chn)
					     .attribute("gain_control_mode")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .optionsAttribute("gain_control_mode_available")
					     .title("Gain Control Mode")
					     .buildSingle();
	layout->addWidget(gainControlMode);
	connect(this, &AD936xHelper::readRequested, gainControlMode, &IIOWidget::readAsync);

	connect(dynamic_cast<ComboAttrUi *>(gainControlMode->getUiStrategy()), &ComboAttrUi::displayedNewData, this,
		[this, hardwaregain, rssi](QString data, QString optionalData) {
			if(data == "manual") {
				hardwaregain->setEnabled(true);
			} else {
				hardwaregain->setEnabled(false);
			}
			hardwaregain->readAsync();
			rssi->readAsync();
		});

	return rxWidget;
}

QWidget *AD936xHelper::generateTxDeviceWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QGridLayout *layout = new QGridLayout(widget);
	layout->setSpacing(10);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel, 0, 0);

	bool isOutput = true;
	iio_channel *altVoltage1 = iio_device_find_channel(dev, "altvoltage1", isOutput);

	// altvoltage1: TX_LO // frequency
	IIOWidget *altVoltage1Frequency = IIOWidgetBuilder(widget)
						  .channel(altVoltage1)
						  .attribute("frequency")
						  .optionsAttribute("frequency_available")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .title("TX LO Frequency(MHz)")
						  .buildSingle();
	altVoltage1Frequency->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	altVoltage1Frequency->setRangeToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	altVoltage1Frequency->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });

	connect(this, &AD936xHelper::readRequested, altVoltage1Frequency, &IIOWidget::readAsync);

	MenuOnOffSwitch *useExternalTxLo = new MenuOnOffSwitch("External Tx LO", widget, false);
	useExternalTxLo->onOffswitch()->setChecked(true);

	layout->addWidget(altVoltage1Frequency, 1, 0, 2, 1);
	layout->addWidget(useExternalTxLo, 2, 0, 1, 1);

	// fastlock profile
	FastlockProfilesWidget *fastlockProfile = new FastlockProfilesWidget(altVoltage1, widget);

	connect(useExternalTxLo->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { fastlockProfile->setEnabled(toggled); });

	connect(fastlockProfile, &FastlockProfilesWidget::recallCalled, this,
		[=, this] { altVoltage1Frequency->read(); });

	layout->addWidget(fastlockProfile, 1, 1, 2, 1);

	return widget;
}

QWidget *AD936xHelper::generateTxChannelWidget(iio_channel *chn, QString title, QWidget *parent)
{
	QWidget *txWidget = new QWidget(parent);
	Style::setStyle(txWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(txWidget);
	txWidget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, txWidget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	// adi,tx-attenuation-mdB
	IIOWidget *txAttenuation = IIOWidgetBuilder(txWidget)
					   .channel(chn)
					   .attribute("hardwaregain")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .optionsAttribute("hardwaregain_available")
					   .title("Attenuation(dB)")
					   .buildSingle();
	layout->addWidget(txAttenuation);
	connect(this, &AD936xHelper::readRequested, txAttenuation, &IIOWidget::readAsync);

	txAttenuation->setDataToUIConversion([this](QString data) {
		// data has dB as string in the value
		auto result = data.split(" ");
		return result.first();
	});

	IIOWidget *rssi = IIOWidgetBuilder(txWidget).channel(chn).attribute("rssi").title("RSSI(dB)").buildSingle();
	layout->addWidget(rssi);
	rssi->setEnabled(false);
	connect(this, &AD936xHelper::readRequested, rssi, &IIOWidget::readAsync);

	return txWidget;
}
