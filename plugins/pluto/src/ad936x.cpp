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

#include "ad936x.h"

#include "fastlockprofileswidget.h"
#include "firfilterqwidget.h"

#include <QLabel>
#include <QTabWidget>
#include <menucombo.h>
#include <toolbuttons.h>
#include <utils.h>
#include <QList>
#include <style.h>
#include <menuonoffswitch.h>
#include <QFutureWatcher>
#include <QtConcurrent>

#include <guistrategy/comboguistrategy.h>

using namespace scopy;
using namespace pluto;

AD936X::AD936X(iio_context *ctx, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
{

	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	m_mainLayout->addWidget(m_tool);

	m_refreshButton = new AnimatedRefreshBtn(false, this);
	m_tool->addWidgetToTopContainerHelper(m_refreshButton, TTA_RIGHT);

	connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
		m_refreshButton->startAnimation();

		QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
		connect(
			watcher, &QFutureWatcher<void>::finished, this,
			[this, watcher]() {
				m_refreshButton->stopAnimation();
				watcher->deleteLater();
			},
			Qt::QueuedConnection);

		QFuture<void> future = QtConcurrent::run([this]() { Q_EMIT readRequested(); });

		watcher->setFuture(future);
	});

	QStackedWidget *centralWidget = new QStackedWidget(this);

	m_controlsWidget = new QWidget(this);
	QVBoxLayout *controlsLayout = new QVBoxLayout(m_controlsWidget);
	controlsLayout->setMargin(0);
	controlsLayout->setContentsMargins(0, 0, 0, 0);
	m_controlsWidget->setLayout(controlsLayout);

	QWidget *controlsWidget = new QWidget(this);
	QVBoxLayout *controlWidgetLayout = new QVBoxLayout(controlsWidget);
	controlWidgetLayout->setMargin(0);
	controlWidgetLayout->setContentsMargins(0, 0, 0, 0);
	controlsWidget->setLayout(controlWidgetLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsWidget);

	controlsLayout->addWidget(scrollArea);

	if(m_ctx != nullptr) {
		///  first widget the global settings can be created with iiowigets only
		controlWidgetLayout->addWidget(generateGlobalSettingsWidget(controlsWidget));

		/// second is Rx ( receive chain)
		controlWidgetLayout->addWidget(generateRxChainWidget(controlsWidget));

		/// third is Tx (transimt chain)
		controlWidgetLayout->addWidget(generateTxChainWidget(controlsWidget));

		controlWidgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	}

	m_blockDiagramWidget = new QWidget(this);
	Style::setBackgroundColor(m_blockDiagramWidget, json::theme::background_primary);
	QVBoxLayout *blockDiagramLayout = new QVBoxLayout(m_blockDiagramWidget);
	m_blockDiagramWidget->setLayout(blockDiagramLayout);

	QWidget *blockDiagramWidget = new QWidget(this);
	QVBoxLayout *blockDiagramWidgetLayout = new QVBoxLayout(blockDiagramWidget);
	blockDiagramWidget->setLayout(blockDiagramWidgetLayout);

	QScrollArea *blockDiagramWidgetScrollArea = new QScrollArea(this);
	blockDiagramWidgetScrollArea->setWidgetResizable(true);
	blockDiagramWidgetScrollArea->setWidget(blockDiagramWidget);

	blockDiagramLayout->addWidget(blockDiagramWidgetScrollArea);

	QLabel *blockDiagram = new QLabel(m_blockDiagramWidget);
	blockDiagramWidgetLayout->addWidget(blockDiagram);
	blockDiagram->setAlignment(Qt::AlignCenter);
	QPixmap pixmap(":/pluto/ad936x.svg");
	blockDiagram->setPixmap(pixmap);

	centralWidget->addWidget(m_controlsWidget);
	centralWidget->addWidget(m_blockDiagramWidget);

	m_tool->addWidgetToCentralContainerHelper(centralWidget);

	QButtonGroup *centralWidgetButtons = new QButtonGroup(this);
	centralWidgetButtons->setExclusive(true);

	QPushButton *ad963xBtn = new QPushButton("Controls", this);
	ad963xBtn->setCheckable(true);
	ad963xBtn->setChecked(true);
	Style::setStyle(ad963xBtn, style::properties::button::blueGrayButton);
	connect(ad963xBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(m_controlsWidget); });

	QPushButton *blockDiagramBtn = new QPushButton("Block Diagram", this);
	blockDiagramBtn->setCheckable(true);
	Style::setStyle(blockDiagramBtn, style::properties::button::blueGrayButton);
	connect(blockDiagramBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(m_blockDiagramWidget); });

	centralWidgetButtons->addButton(ad963xBtn);
	centralWidgetButtons->addButton(blockDiagramBtn);

	m_tool->addWidgetToTopContainerHelper(ad963xBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(blockDiagramBtn, TTA_LEFT);
}

AD936X::~AD936X() {}

QWidget *AD936X::generateGlobalSettingsWidget(QWidget *parent)
{
	QWidget *globalSettingsWidget = new QWidget(parent);
	Style::setBackgroundColor(globalSettingsWidget, json::theme::background_primary);
	Style::setStyle(globalSettingsWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(globalSettingsWidget);
	globalSettingsWidget->setLayout(layout);

	QLabel *title = new QLabel("AD9361 / AD9364 Global Settings", globalSettingsWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	iio_device *plutoDevice = iio_context_find_device(m_ctx, "ad9361-phy");

	QHBoxLayout *hlayout = new QHBoxLayout();

	//// ensm_mode
	IIOWidget *ensmMode = IIOWidgetBuilder(globalSettingsWidget)
				      .device(plutoDevice)
				      .attribute("ensm_mode")
				      .optionsAttribute("ensm_mode_available")
				      .title("ENSM Mode")
				      .uiStrategy(IIOWidgetBuilder::ComboUi)
				      .buildSingle();

	hlayout->addWidget(ensmMode);

	connect(this, &AD936X::readRequested, ensmMode, &IIOWidget::readAsync);

	////calib_mode
	IIOWidget *calibMode = IIOWidgetBuilder(globalSettingsWidget)
				       .device(plutoDevice)
				       .attribute("calib_mode")
				       .optionsAttribute("calib_mode_available")
				       .title("Calibration Mode")
				       .uiStrategy(IIOWidgetBuilder::ComboUi)
				       .buildSingle();
	hlayout->addWidget(calibMode);
	connect(this, &AD936X::readRequested, calibMode, &IIOWidget::readAsync);

	Style::setStyle(calibMode, style::properties::widget::basicBackground, true, true);

	// trx_rate_governor
	IIOWidget *trxRateGovernor = IIOWidgetBuilder(globalSettingsWidget)
					     .device(plutoDevice)
					     .attribute("trx_rate_governor")
					     .optionsAttribute("trx_rate_governor_available")
					     .title("TRX Rate Governor")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .buildSingle();
	hlayout->addWidget(trxRateGovernor);
	connect(this, &AD936X::readRequested, trxRateGovernor, &IIOWidget::readAsync);

	FirFilterQWidget *firFilter = new FirFilterQWidget(plutoDevice, nullptr, globalSettingsWidget);
	hlayout->addWidget(firFilter);

	layout->addLayout(hlayout);

	// rx_path_rates
	IIOWidget *rxPathRates = IIOWidgetBuilder(globalSettingsWidget)
					 .device(plutoDevice)
					 .attribute("rx_path_rates")
					 .title("RX Path Rates")
					 .buildSingle();
	layout->addWidget(rxPathRates);
	rxPathRates->setEnabled(false);
	connect(this, &AD936X::readRequested, rxPathRates, &IIOWidget::readAsync);

	// tx_path_rates
	IIOWidget *txPathRates = IIOWidgetBuilder(globalSettingsWidget)
					 .device(plutoDevice)
					 .attribute("tx_path_rates")
					 .title("Tx Path Rates")
					 .buildSingle();
	layout->addWidget(txPathRates);
	txPathRates->setEnabled(false);
	connect(this, &AD936X::readRequested, txPathRates, &IIOWidget::readAsync);

	connect(firFilter, &FirFilterQWidget::filterChanged, this, [=, this]() {
		rxPathRates->read();
		txPathRates->read();
	});

	// xo_correction
	IIOWidget *xoCorrection = IIOWidgetBuilder(globalSettingsWidget)
					  .device(plutoDevice)
					  .attribute("xo_correction")
					  .optionsAttribute("xo_correction_available")
					  .title("XO Correction")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .buildSingle();
	layout->addWidget(xoCorrection);
	connect(this, &AD936X::readRequested, xoCorrection, &IIOWidget::readAsync);

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return globalSettingsWidget;
}

QWidget *AD936X::generateRxChainWidget(QWidget *parent)
{
	QWidget *rxChainWidget = new QWidget(parent);
	Style::setBackgroundColor(rxChainWidget, json::theme::background_primary);
	Style::setStyle(rxChainWidget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(rxChainWidget);
	rxChainWidget->setLayout(mainLayout);

	QLabel *title = new QLabel("AD9361 / AD9364 Receive Chain", rxChainWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	mainLayout->addWidget(title);

	iio_device *plutoDevice = iio_context_find_device(m_ctx, "ad9361-phy");

	QGridLayout *layout = new QGridLayout();

	bool isOutput = false;

	iio_channel *voltage0 = iio_device_find_channel(plutoDevice, "voltage0", isOutput);

	// voltage0: rf_bandwidth
	IIOWidget *rfBandwidth = IIOWidgetBuilder(rxChainWidget)
					 .channel(voltage0)
					 .attribute("rf_bandwidth")
					 .optionsAttribute("rf_bandwidth_available")
					 .title("RF Bandwidth(MHz)")
					 .uiStrategy(IIOWidgetBuilder::RangeUi)
					 .buildSingle();
	layout->addWidget(rfBandwidth, 0, 0, 2, 1);
	connect(this, &AD936X::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// voltage0:  sampling_frequency
	IIOWidget *samplingFrequency = IIOWidgetBuilder(rxChainWidget)
					       .channel(voltage0)
					       .attribute("sampling_frequency")
					       .optionsAttribute("sampling_frequency_available")
					       .title("Sampling Rate(MSPS)")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .buildSingle();
	layout->addWidget(samplingFrequency, 0, 1, 2, 1);
	connect(this, &AD936X::readRequested, samplingFrequency, &IIOWidget::readAsync);

	// voltage 0 : rf_port_select
	IIOWidget *rfPortSelect = IIOWidgetBuilder(rxChainWidget)
					  .channel(voltage0)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .title("RF Port Select")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .buildSingle();
	layout->addWidget(rfPortSelect, 0, 2, 2, 1);
	connect(this, &AD936X::readRequested, rfPortSelect, &IIOWidget::readAsync);

	// because this channel is marked as output by libiio we need to mark altvoltage0 as output
	iio_channel *altVoltage0 = iio_device_find_channel(plutoDevice, "altvoltage0", true);

	// altvoltage0: RX_LO // frequency
	IIOWidget *altVoltage0Frequency = IIOWidgetBuilder(rxChainWidget)
						  .channel(altVoltage0)
						  .attribute("frequency")
						  .optionsAttribute("frequency_available")
						  .title("RX LO Frequency(MHz)")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .buildSingle();
	connect(this, &AD936X::readRequested, altVoltage0Frequency, &IIOWidget::readAsync);

	MenuOnOffSwitch *useExternalRxLo = new MenuOnOffSwitch("External Rx LO", rxChainWidget, false);
	useExternalRxLo->onOffswitch()->setChecked(true);

	layout->addWidget(altVoltage0Frequency, 0, 3, 2, 1);
	layout->addWidget(useExternalRxLo, 2, 3, 1, 1);

	// fastlock profile
	FastlockProfilesWidget *fastlockProfile = new FastlockProfilesWidget(altVoltage0, rxChainWidget);

	connect(useExternalRxLo->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { fastlockProfile->setEnabled(toggled); });

	layout->addWidget(fastlockProfile, 0, 4, 3, 1);

	connect(fastlockProfile, &FastlockProfilesWidget::recallCalled, this,
		[=, this] { altVoltage0Frequency->read(); });

	// quadrature_tracking_en
	IIOWidget *quadratureTrackingEn = IIOWidgetBuilder(rxChainWidget)
						  .channel(voltage0)
						  .attribute("quadrature_tracking_en")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("Quadrature")
						  .buildSingle();
	layout->addWidget(quadratureTrackingEn, 0, 5);
	quadratureTrackingEn->showProgressBar(false);
	connect(this, &AD936X::readRequested, quadratureTrackingEn, &IIOWidget::readAsync);

	// rf_dc_offset_tracking_en
	IIOWidget *rcDcOffsetTrackingEn = IIOWidgetBuilder(rxChainWidget)
						  .channel(voltage0)
						  .attribute("rf_dc_offset_tracking_en")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("RF DC")
						  .buildSingle();
	layout->addWidget(rcDcOffsetTrackingEn, 1, 5);
	rcDcOffsetTrackingEn->showProgressBar(false);
	connect(this, &AD936X::readRequested, rcDcOffsetTrackingEn, &IIOWidget::readAsync);

	// bb_dc_offset_tracking_en
	IIOWidget *bbDcOffsetTrackingEn = IIOWidgetBuilder(rxChainWidget)
						  .channel(voltage0)
						  .attribute("bb_dc_offset_tracking_en")
						  .title("BB DC")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .buildSingle();
	layout->addWidget(bbDcOffsetTrackingEn, 2, 5);
	bbDcOffsetTrackingEn->showProgressBar(false);
	connect(this, &AD936X::readRequested, bbDcOffsetTrackingEn, &IIOWidget::readAsync);

	mainLayout->addLayout(layout);

	QHBoxLayout *rxWidgetsLayout = new QHBoxLayout();
	rxWidgetsLayout->addWidget(generateRxWidget(voltage0, rxChainWidget));

	// TODO add condition for multiple devices

	rxWidgetsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));
	mainLayout->addLayout(rxWidgetsLayout);

	mainLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return rxChainWidget;
}

QWidget *AD936X::generateRxWidget(iio_channel *chn, QWidget *parent)
{
	QWidget *rxWidget = new QWidget(parent);
	Style::setStyle(rxWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(rxWidget);
	rxWidget->setLayout(layout);

	layout->addWidget(new QLabel("RX", rxWidget));

	// voltage0: hardwaregain
	IIOWidget *hardwaregain = IIOWidgetBuilder(rxWidget)
					  .channel(chn)
					  .attribute("hardwaregain")
					  .uiStrategy(IIOWidgetBuilder::RangeUi)
					  .optionsAttribute("hardwaregain_available")
					  .title("Hardware Gain(dB)")
					  .buildSingle();
	layout->addWidget(hardwaregain);
	connect(this, &AD936X::readRequested, hardwaregain, &IIOWidget::readAsync);

	hardwaregain->setDataToUIConversion([this](QString data) {
		auto result = data.split(" ");
		return result.first();
	});

	hardwaregain->lastReturnCode();

	// voltage: rssi
	IIOWidget *rssi = IIOWidgetBuilder(rxWidget).channel(chn).attribute("rssi").title("RSSI(dB)").buildSingle();
	layout->addWidget(rssi);
	rssi->setEnabled(false);

	// voltage: gain_control_mode
	IIOWidget *gainControlMode = IIOWidgetBuilder(rxWidget)
					     .channel(chn)
					     .attribute("gain_control_mode")
					     .uiStrategy(IIOWidgetBuilder::ComboUi)
					     .optionsAttribute("gain_control_mode_available")
					     .title("Gain Control Mode")
					     .buildSingle();
	layout->addWidget(gainControlMode);
	connect(this, &AD936X::readRequested, gainControlMode, &IIOWidget::readAsync);

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

QWidget *AD936X::generateTxChainWidget(QWidget *parent)
{
	QWidget *txChainWidget = new QWidget(parent);
	Style::setBackgroundColor(txChainWidget, json::theme::background_primary);
	Style::setStyle(txChainWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(txChainWidget);
	txChainWidget->setLayout(layout);

	QLabel *title = new QLabel("AD9361 / AD9364 Transmit Chain", txChainWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	iio_device *plutoDevice = iio_context_find_device(m_ctx, "ad9361-phy");

	QGridLayout *lay = new QGridLayout();

	bool isOutput = true;
	iio_channel *voltage0 = iio_device_find_channel(plutoDevice, "voltage0", isOutput);

	// voltage0: rf_bandwidth
	IIOWidget *rfBandwidth = IIOWidgetBuilder(txChainWidget)
					 .channel(voltage0)
					 .attribute("rf_bandwidth")
					 .optionsAttribute("rf_bandwidth_available")
					 .uiStrategy(IIOWidgetBuilder::RangeUi)
					 .buildSingle();
	lay->addWidget(rfBandwidth, 0, 0, 2, 1);
	connect(this, &AD936X::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// voltage0:  sampling_frequency
	IIOWidget *samplingFrequency = IIOWidgetBuilder(txChainWidget)
					       .channel(voltage0)
					       .attribute("sampling_frequency")
					       .optionsAttribute("sampling_frequency_available")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .buildSingle();
	lay->addWidget(samplingFrequency, 0, 1, 2, 1);
	connect(this, &AD936X::readRequested, samplingFrequency, &IIOWidget::readAsync);

	// voltage0:  rf_port_select
	IIOWidget *rfPortSelect = IIOWidgetBuilder(txChainWidget)
					  .channel(voltage0)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .buildSingle();
	lay->addWidget(rfPortSelect, 0, 2, 2, 1);
	connect(this, &AD936X::readRequested, rfPortSelect, &IIOWidget::readAsync);

	iio_channel *altVoltage1 = iio_device_find_channel(plutoDevice, "altvoltage1", isOutput);

	// altvoltage1: TX_LO // frequency
	IIOWidget *altVoltage1Frequency = IIOWidgetBuilder(txChainWidget)
						  .channel(altVoltage1)
						  .attribute("frequency")
						  .optionsAttribute("frequency_available")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .buildSingle();
	connect(this, &AD936X::readRequested, altVoltage1Frequency, &IIOWidget::readAsync);

	MenuOnOffSwitch *useExternalTxLo = new MenuOnOffSwitch("External Tx LO", txChainWidget, false);
	useExternalTxLo->onOffswitch()->setChecked(true);

	lay->addWidget(altVoltage1Frequency, 0, 3, 2, 1);
	lay->addWidget(useExternalTxLo, 2, 3, 1, 1);

	// fastlock profile
	FastlockProfilesWidget *fastlockProfile = new FastlockProfilesWidget(altVoltage1, txChainWidget);

	connect(useExternalTxLo->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { fastlockProfile->setEnabled(toggled); });

	lay->addWidget(fastlockProfile, 0, 4, 3, 1);

	connect(fastlockProfile, &FastlockProfilesWidget::recallCalled, this,
		[=, this] { altVoltage1Frequency->read(); });

	layout->addLayout(lay);

	QHBoxLayout *txWidgetsLayout = new QHBoxLayout();
	txWidgetsLayout->addWidget(generateTxWidget(plutoDevice, txChainWidget));

	// TODO add condition for multiple devices

	txWidgetsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));
	layout->addLayout(txWidgetsLayout);

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return txChainWidget;
}

QWidget *AD936X::generateTxWidget(iio_device *dev, QWidget *parent)
{
	QWidget *txWidget = new QWidget(parent);
	Style::setStyle(txWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(txWidget);
	txWidget->setLayout(layout);

	layout->addWidget(new QLabel("TX", txWidget));

	// adi,tx-attenuation-mdB
	IIOWidget *txAttenuation = IIOWidgetBuilder(txWidget)
					   .device(dev)
					   .attribute("adi,tx-attenuation-mdB")
					   .title("Attenuation(dB)")
					   .buildSingle();
	layout->addWidget(txAttenuation);
	connect(this, &AD936X::readRequested, txAttenuation, &IIOWidget::readAsync);

	txAttenuation->setUItoDataConversion([this](QString data) {
		double value = data.toDouble() * 1000;
		return QString::number(value);
	});
	txAttenuation->setDataToUIConversion([this](QString data) {
		double value = data.toDouble() / 1000;
		return QString::number(value);
	});

	bool isOutput = true;
	iio_channel *voltage0 = iio_device_find_channel(dev, "voltage0", isOutput);
	// voltage0: rssi
	IIOWidget *rssi =
		IIOWidgetBuilder(txWidget).channel(voltage0).attribute("rssi").title("RSSI(dB)").buildSingle();
	layout->addWidget(rssi);
	rssi->setEnabled(false);
	connect(this, &AD936X::readRequested, rssi, &IIOWidget::readAsync);

	return txWidget;
}
