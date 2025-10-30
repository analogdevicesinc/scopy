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

#include "adrv9002.h"
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/guistrategy/temperatureguistrategy.h>

#include <QLoggingCategory>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <style.h>
#include <stylehelper.h>

Q_LOGGING_CATEGORY(CAT_ADRV9002, "ADRV9002")

using namespace scopy::adrv9002;
using namespace scopy;

Adrv9002::Adrv9002(iio_context *ctx, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_tool(nullptr)
	, m_refreshButton(nullptr)
	, m_scrollArea(nullptr)
	, m_centralWidget(nullptr)
	, m_iio_dev(nullptr)
	, m_profileManager(nullptr)
	, m_initialCalibrationsWidget(nullptr)
{
	setupUi();
	connectSignals();
}

Adrv9002::~Adrv9002() {}

void Adrv9002::setupUi()
{
	// Standard Scopy tool layout pattern
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

	if(!m_ctx) {
		qWarning(CAT_ADRV9002) << "No context provided";
		return;
	}

	// Find ADRV9002 family device - same devices as in compatible() function
	const char *adrv9002_devices[] = {"adrv9002-phy", "adrv9003-phy", "adrv9004-phy", "adrv9005-phy",
					  "adrv9006-phy"};

	for(const char *device_name : adrv9002_devices) {
		m_iio_dev = iio_context_find_device(m_ctx, device_name);
		if(m_iio_dev) {
			qDebug(CAT_ADRV9002) << "Found ADRV9002 device:" << device_name;
			break;
		}
	}

	if(!m_iio_dev) {
		qWarning(CAT_ADRV9002) << "No ADRV9002 device found in context";
		return;
	}

	// Initialize profile manager
	m_profileManager = new ProfileManager(m_iio_dev, this);

	// Initialize initial calibrations widget (only if supported)
	m_initialCalibrationsWidget = new InitialCalibrationsWidget(m_iio_dev, this);
	m_initialCalibrationsWidget->setEnabled(InitialCalibrationsWidget::isSupported(m_iio_dev));

	// Create tab system
	m_tabCentralWidget = new QStackedWidget(this);

	// Create the three tab widgets
	m_controlsWidget = createControlsWidget();
	m_profileGeneratorWidget = createProfileGeneratorWidget();

	// Add widgets to stack
	m_tabCentralWidget->addWidget(m_controlsWidget);
	m_tabCentralWidget->addWidget(m_profileGeneratorWidget);

	// Setup tab buttons
	setupTabButtons();

	// Add stacked widget to central container
	m_tool->addWidgetToCentralContainerHelper(m_tabCentralWidget);

	// Set default tab
	m_controlsBtn->setChecked(true);
	m_tabCentralWidget->setCurrentWidget(m_controlsWidget);
}

void Adrv9002::connectSignals()
{
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
}

void Adrv9002::setupTabButtons()
{
	// Create button group
	m_tabButtons = new QButtonGroup(this);
	m_tabButtons->setExclusive(true);

	// Create tab buttons
	m_controlsBtn = new QPushButton("Controls", this);
	m_profileGeneratorBtn = new QPushButton("Profile Generator", this);

	// Make buttons checkable and style them
	m_controlsBtn->setCheckable(true);
	m_profileGeneratorBtn->setCheckable(true);

	Style::setStyle(m_controlsBtn, style::properties::button::blueGrayButton);
	Style::setStyle(m_profileGeneratorBtn, style::properties::button::blueGrayButton);

	// Connect tab switching
	connect(m_controlsBtn, &QPushButton::clicked, this,
		[=, this]() { m_tabCentralWidget->setCurrentWidget(m_controlsWidget); });
	connect(m_profileGeneratorBtn, &QPushButton::clicked, this,
		[=, this]() { m_tabCentralWidget->setCurrentWidget(m_profileGeneratorWidget); });

	// Add to button group
	m_tabButtons->addButton(m_controlsBtn);
	m_tabButtons->addButton(m_profileGeneratorBtn);

	// Add buttons to top container
	m_tool->addWidgetToTopContainerHelper(m_controlsBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_profileGeneratorBtn, TTA_LEFT);
}

void Adrv9002::createControls(QWidget *centralWidget)
{
	// Create main layout with ad936x pattern (no styling on central widget)
	QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
	mainLayout->setMargin(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(10); // Add spacing between sections

	if(m_ctx && m_iio_dev) {
		// Add Device Driver API section first (matching iio-oscilloscope layout)
		mainLayout->addWidget(generateDeviceDriverAPIWidget(centralWidget));

		// Create collapsible section widgets
		mainLayout->addWidget(createGlobalSettingsSection(centralWidget));
		mainLayout->addWidget(createReceiveChainSection(centralWidget));
		mainLayout->addWidget(createTransmitChainSection(centralWidget));
	}

	// Add spacer to push content to top
	mainLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

QWidget *Adrv9002::createControlsWidget()
{
	// Create scrollable central widget for Controls tab
	m_centralWidget = new QWidget();
	m_scrollArea = new QScrollArea();
	m_scrollArea->setWidget(m_centralWidget);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// Create controls in the central widget (current implementation)
	createControls(m_centralWidget);

	return m_scrollArea;
}

QWidget *Adrv9002::createProfileGeneratorWidget()
{
	// Create container widget following prompt pattern
	QWidget *container = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(container);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 0);

	// Create scrollable area following prompt #3 pattern
	QScrollArea *scrollArea = new QScrollArea();
	ProfileGeneratorWidget *profileGen = new ProfileGeneratorWidget(m_iio_dev, this);

	scrollArea->setWidget(profileGen);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	layout->addWidget(scrollArea);
	return container;
}

void Adrv9002::reloadSettings() { Q_EMIT readRequested(); }

// UI Section Creation Functions

QWidget *Adrv9002::generateDeviceDriverAPIWidget(QWidget *parent)
{
	QWidget *driverAPIWidget = new QWidget(parent);
	Style::setBackgroundColor(driverAPIWidget, json::theme::background_primary);
	Style::setStyle(driverAPIWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(driverAPIWidget);
	layout->setMargin(5);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(2);

	// Title
	QLabel *title = new QLabel("Device Driver API", driverAPIWidget);
	Style::setStyle(title, style::properties::label::menuSmall);
	layout->addWidget(title);

	// Version label - simple display matching iio-oscilloscope
	QString version = getDeviceDriverVersion();
	QLabel *versionLabel = new QLabel(version, driverAPIWidget);
	Style::setStyle(versionLabel, style::properties::label::subtle);
	layout->addWidget(versionLabel);

	return driverAPIWidget;
}

QString Adrv9002::getDeviceDriverVersion()
{
	// Try to get version from context or device attributes
	if(m_ctx) {
		char api_version[16];
		auto ret = iio_device_debug_attr_read(m_iio_dev, "api_version", api_version, sizeof(api_version));
		if(ret < 0) {
			return "";
		} else {
			return QString(api_version);
		}
	}

	return "";
}

MenuSectionCollapseWidget *Adrv9002::createGlobalSettingsSection(QWidget *parent)
{
	auto section = new MenuSectionCollapseWidget("ADRV9002 Global Settings", MenuCollapseSection::MHCW_ARROW,
						     MenuCollapseSection::MHW_BASEWIDGET, parent);

	// Profile Manager - handles profile_config and stream_config
	if(m_profileManager) {
		section->add(m_profileManager);
		connect(this, &Adrv9002::readRequested, m_profileManager, &ProfileManager::updateDeviceInfo);
	}

	QWidget *widget = new QWidget(section);

	QHBoxLayout *layout = new QHBoxLayout(widget);
	layout->setSpacing(15);
	layout->setMargin(0);
	widget->setLayout(layout);

	// Temperature (separate from profile management)
	iio_channel *tempCh = iio_device_find_channel(m_iio_dev, "temp0", false);
	if(tempCh) {
		IIOWidget *tempWidget = IIOWidgetBuilder(this)
						.device(m_iio_dev)
						.channel(tempCh)
						.attribute("input")
						.uiStrategy(IIOWidgetBuilder::TemperatureUi)
						.title("Temperature")
						.buildSingle();

		if(tempWidget) {
			tempWidget->showProgressBar(false);
			// Access the TemperatureGuiStrategy to set critical temperature
			auto *tempStrategy = dynamic_cast<TemperatureGuiStrategy *>(tempWidget->getUiStrategy());
			if(tempStrategy) {
				tempStrategy->setCriticalTemperature(
					80.0, "ADRV9002 temperature critical! Risk of thermal shutdown.");
				tempStrategy->setWarningOffset(5.0); // Warn at 75°C (80°C - 5°C)
				// Optional: Configure periodic updates (default is 5 seconds)
				// tempStrategy->setPeriodicUpdates(true, 5); // Update every 5 second
			}

			connect(this, &Adrv9002::readRequested, tempWidget, &IIOWidget::readAsync);

			layout->addWidget(tempWidget);
		}
	}

	// Initial Calibrations section (if supported)
	if(m_initialCalibrationsWidget) {
		layout->addWidget(m_initialCalibrationsWidget);
		connect(this, &Adrv9002::readRequested, m_initialCalibrationsWidget,
			&InitialCalibrationsWidget::refreshStatus);
	}

	section->add(widget);

	return section;
}

MenuSectionCollapseWidget *Adrv9002::createReceiveChainSection(QWidget *parent)
{
	auto section = new MenuSectionCollapseWidget("ADRV9002 Receive Chain", MenuCollapseSection::MHCW_ARROW,
						     MenuCollapseSection::MHW_BASEWIDGET, parent);

	// Create container widget for RX channels layout
	QWidget *rxContainer = new QWidget();
	QVBoxLayout *containerLayout = new QVBoxLayout(rxContainer);
	containerLayout->setMargin(0);
	containerLayout->setContentsMargins(0, 0, 0, 0);

	// Main RX section with RX1 and RX2 side by side
	QHBoxLayout *rxLayout = new QHBoxLayout();
	rxLayout->setSpacing(15);

	// RX1 Column
	rxLayout->addWidget(createRxChannelControls("RX 1", 0));

	// RX2 Column
	rxLayout->addWidget(createRxChannelControls("RX 2", 1));

	containerLayout->addLayout(rxLayout);

	// ORX Section below RX1/RX2
	QHBoxLayout *orxLayout = new QHBoxLayout();
	orxLayout->addWidget(createOrxControls());
	containerLayout->addLayout(orxLayout);

	section->add(rxContainer);

	return section;
}

MenuSectionCollapseWidget *Adrv9002::createTransmitChainSection(QWidget *parent)
{
	auto section = new MenuSectionCollapseWidget("ADRV9002 Transmit Chain", MenuCollapseSection::MHCW_ARROW,
						     MenuCollapseSection::MHW_BASEWIDGET, parent);

	// Create container widget for TX channels layout
	QWidget *txContainer = new QWidget();
	QHBoxLayout *txLayout = new QHBoxLayout(txContainer);
	txLayout->setMargin(0);
	txLayout->setContentsMargins(0, 0, 0, 0);
	txLayout->setSpacing(15);

	// TX1 Column
	txLayout->addWidget(createTxChannelControls("TX 1", 0));

	// TX2 Column
	txLayout->addWidget(createTxChannelControls("TX 2", 1));

	section->add(txContainer);

	return section;
}

// Channel Control Creation Functions

QWidget *Adrv9002::createRxChannelControls(const QString &title, int channel)
{
	QWidget *widget = new QWidget();
	// Add individual channel styling for borders
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QGridLayout *layout = new QGridLayout(widget);
	layout->setSpacing(10);
	layout->setContentsMargins(15, 15, 15, 15);

	// Channel title spanning full width
	QLabel *titleLabel = new QLabel(title);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel, 0, 0, 1, 2); // Span 2 columns

	// Find channels
	QString channelName = QString("voltage%1").arg(channel);
	iio_channel *rxCh = iio_device_find_channel(m_iio_dev, channelName.toLocal8Bit().data(), false);
	QString loChannelName = QString("altvoltage%1").arg(channel);
	iio_channel *loCh = iio_device_find_channel(m_iio_dev, loChannelName.toLocal8Bit().data(), true);

	if(!rxCh) {
		QLabel *errorLabel = new QLabel("Channel not found");
		layout->addWidget(errorLabel);
		return widget;
	}

	// Column 0 (Left) - from iio-oscilloscope column 2
	layout->addWidget(createRangeWidget(rxCh, "hardwaregain", "[0 0.5 36]", "Hardware Gain (dB)"), 1, 0);
	layout->addWidget(createComboWidget(rxCh, "gain_control_mode", "gain_control_mode_available", "Gain Control"),
			  2, 0);
	layout->addWidget(createComboWidget(rxCh, "ensm_mode", "ensm_mode_available", "ENSM"), 3, 0);
	IIOWidget *rxEn = createCheckboxWidget(rxCh, "en", "Powerdown");
	// logic needed to match functionality
	rxEn->setDataToUIConversion([](QString data) {
		if(data == "0") {
			return "1";
		}
		return "0";
	});
	rxEn->setUItoDataConversion([](QString data) {
		if(data == "0") {
			return "1";
		}
		return "0";
	});
	layout->addWidget(rxEn, 4, 0);

	layout->addWidget(createCheckboxWidget(rxCh, "bbdc_rejection_en", "BBDC Rejection"), 5, 0);

	// Only create NCO widget if supported
	double dummy;
	int ret = iio_channel_attr_read_double(rxCh, "nco_frequency", &dummy);
	if(ret == 0) {
		layout->addWidget(createRangeWidget(rxCh, "nco_frequency", "[-20000 1 20000]", "NCO (Hz)"), 6, 0);
	}

	layout->addWidget(createReadOnlyWidget(rxCh, "decimated_power", "Decimated Power (dB)"), 7, 0);
	IIOWidget *rfBandwidth = createReadOnlyWidget(rxCh, "rf_bandwidth", "Bandwidth (MHz)");
	rfBandwidth->setDataToUIConversion([](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 4); });
	rfBandwidth->setUItoDataConversion([](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 4); });
	layout->addWidget(rfBandwidth, 8, 0);

	// Column 1 (Right) - from iio-oscilloscope column 4
	layout->addWidget(createComboWidget(rxCh, "digital_gain_control_mode", "digital_gain_control_mode_available",
					    "Digital Gain Control"),
			  1, 1);
	layout->addWidget(createComboWidget(rxCh, "interface_gain", "interface_gain_available", "Interface Gain (dB)"),
			  2, 1);
	layout->addWidget(createComboWidget(rxCh, "port_en_mode", "port_en_mode_available", "Port Enable"), 3, 1);
	layout->addWidget(createCheckboxWidget(rxCh, "dynamic_adc_switch_en", "Dynamic Adc Switch"), 4, 1);

	//////   BBDC Loop Gain ////
	static constexpr uint32_t BBDC_LOOP_GAIN_RES = 2147483648U; // 2^31
	static const double bbdc_adjust_min = 1.0 / BBDC_LOOP_GAIN_RES;
	static const double bbdc_adjust_max = 1.0 / BBDC_LOOP_GAIN_RES * UINT32_MAX;
	QString bbdcRange = QString("[%1 0.0001 %2]").arg(bbdc_adjust_min, 0, 'f', 15).arg(bbdc_adjust_max, 0, 'f', 15);

	IIOWidget *bbdcWidget = createRangeWidget(rxCh, "bbdc_loop_gain_raw", bbdcRange, "BBDC Loop Gain (dB)");
	// Add conversion functions similar to iio-oscilloscope
	bbdcWidget->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / BBDC_LOOP_GAIN_RES, 'f', 15); });
	bbdcWidget->setUItoDataConversion(
		[](QString data) { return QString::number(round(data.toDouble() * BBDC_LOOP_GAIN_RES), 'f', 0); });
	layout->addWidget(bbdcWidget, 5, 1);

	if(loCh) {
		QString loAttr = QString("RX%1_LO_frequency").arg(channel + 1);
		auto loWidget = createRangeWidget(loCh, loAttr, "[30 1 6000]", "Local Oscillator (MHz)");

		// Add MHz ↔ Hz conversion
		loWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 4); });
		loWidget->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 4); });

		layout->addWidget(loWidget, 6, 1);
	}

	layout->addWidget(createContinuousReadOnlyWidget(rxCh, "rssi", "RSSI (dB)"), 7, 1);

	IIOWidget *samplingFreq = createReadOnlyWidget(rxCh, "sampling_frequency", "Sampling Rate (MSPS)");
	// Add MSPS ↔ SPS conversion
	samplingFreq->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 4); });
	samplingFreq->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 4); });
	layout->addWidget(samplingFreq, 8, 1);

	// Tracking section at bottom spanning both columns
	QLabel *trackingLabel = new QLabel("Tracking:");
	Style::setStyle(trackingLabel, style::properties::label::menuBig);
	layout->addWidget(trackingLabel, 10, 0, 1, 2);

	// Create tracking checkboxes in 2x3 grid layout matching iio-oscilloscope
	layout->addWidget(createCheckboxWidget(rxCh, "quadrature_fic_tracking_en", "Quadrature FIC"), 11, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "agc_tracking_en", "AGC"), 11, 1);
	layout->addWidget(createCheckboxWidget(rxCh, "bbdc_rejection_tracking_en", "BBDC Rejection"), 12, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "quadrature_w_poly_tracking_en", "Quadrature Poly"), 12, 1);
	layout->addWidget(createCheckboxWidget(rxCh, "hd_tracking_en", "HD2"), 13, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "rssi_tracking_en", "RSSI"), 13, 1);

	return widget;
}

QWidget *Adrv9002::createTxChannelControls(const QString &title, int channel)
{
	QWidget *widget = new QWidget();
	// Add individual channel styling for borders
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QGridLayout *layout = new QGridLayout(widget);
	layout->setSpacing(10);
	layout->setContentsMargins(15, 15, 15, 15);

	// Channel title spanning full width
	QLabel *titleLabel = new QLabel(title);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel, 0, 0, 1, 2); // Span 2 columns

	// Find channels
	QString channelName = QString("voltage%1").arg(channel);
	iio_channel *txCh = iio_device_find_channel(m_iio_dev, channelName.toLocal8Bit().data(), true);
	QString loChannelName = QString("altvoltage%1").arg(channel + 2); // TX LO channels are altvoltage2/3
	iio_channel *loCh = iio_device_find_channel(m_iio_dev, loChannelName.toLocal8Bit().data(), true);

	if(!txCh) {
		QLabel *errorLabel = new QLabel("Channel not found");
		layout->addWidget(errorLabel);
		return widget;
	}

	// Column 0 (Left) - from iio-oscilloscope TX column 2
	layout->addWidget(createRangeWidget(txCh, "hardwaregain", "[-41.95 0.05 0]", "Attenuation (dB)"), 1, 0);
	layout->addWidget(
		createComboWidget(txCh, "atten_control_mode", "atten_control_mode_available", "Attenuation Control"), 2,
		0);

	if(loCh) {
		QString loAttr = QString("TX%1_LO_frequency").arg(channel + 1);
		auto loWidget = createRangeWidget(loCh, loAttr, "[30 1 6000]", "Local Oscillator (MHz)");

		// Add MHz ↔ Hz conversion
		loWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 4); });
		loWidget->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 4); });

		layout->addWidget(loWidget, 3, 0);
	}

	// Only create NCO widget if supported
	double dummy;
	int ret = iio_channel_attr_read_double(txCh, "nco_frequency", &dummy);
	if(ret == 0) {
		layout->addWidget(createRangeWidget(txCh, "nco_frequency", "[-20000 1 20000]", "NCO (Hz)"), 4, 0);
	}

	IIOWidget *rfBandwidth = createReadOnlyWidget(txCh, "rf_bandwidth", "Bandwidth (MHz)");

	// Add MHz ↔ Hz conversion
	rfBandwidth->setDataToUIConversion([](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 4); });
	rfBandwidth->setUItoDataConversion([](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 4); });
	layout->addWidget(rfBandwidth, 5, 0);

	// Column 1 (Right) - from iio-oscilloscope TX column 4
	layout->addWidget(createComboWidget(txCh, "port_en_mode", "port_en_mode_available", "Port Enable"), 1, 1);
	layout->addWidget(createComboWidget(txCh, "ensm_mode", "ensm_mode_available", "ENSM"), 2, 1);
	IIOWidget *txEn = createCheckboxWidget(txCh, "en", "Powerdown");
	// logic needed to match functionality
	txEn->setDataToUIConversion([](QString data) {
		if(data == "0") {
			return "1";
		}
		return "0";
	});
	txEn->setUItoDataConversion([](QString data) {
		if(data == "0") {
			return "1";
		}
		return "0";
	});
	layout->addWidget(txEn, 3, 1);
	// Row+3 is empty in iio-oscilloscope
	IIOWidget *samplingFreq = createReadOnlyWidget(txCh, "sampling_frequency", "Sampling Rate (MSPS)");
	// Add MSPS ↔ SPS conversion
	samplingFreq->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 4); });
	samplingFreq->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 4); });
	layout->addWidget(samplingFreq, 5, 1);

	QLabel *trackingLabel = new QLabel("Tracking:");
	Style::setStyle(trackingLabel, style::properties::label::menuBig);
	layout->addWidget(trackingLabel, 7, 0, 1, 2);

	// Create tracking checkboxes in 2x3 grid layout matching iio-oscilloscope
	layout->addWidget(createCheckboxWidget(txCh, "quadrature_tracking_en", "Quadrature"), 8, 0);
	layout->addWidget(createCheckboxWidget(txCh, "close_loop_gain_tracking_en", "Close Loop Gain"), 8, 1);
	layout->addWidget(createCheckboxWidget(txCh, "pa_correction_tracking_en", "PA Correction"), 9, 0);
	layout->addWidget(createCheckboxWidget(txCh, "loopback_delay_tracking_en", "Loopback Delay"), 9, 1);
	layout->addWidget(createCheckboxWidget(txCh, "lo_leakage_tracking_en", "LO Leakage"), 9, 0);

	return widget;
}

QWidget *Adrv9002::createOrxControls()
{
	QWidget *widget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout(widget);
	layout->setSpacing(15);

	// ORX 1 Column
	QWidget *orx1Widget = createOrxChannelControls("ORX 1", 0);

	if(orx1Widget != nullptr)
		layout->addWidget(orx1Widget);

	QString deviceName = QString(iio_device_get_name(m_iio_dev));
	bool showOrx2 = !(deviceName == "adrv9003-phy" || deviceName == "adrv9005-phy");

	if(showOrx2) {
		// ORX 2 Column
		QWidget *orx2Widget = createOrxChannelControls("ORX 2", 1);
		if(orx2Widget != nullptr)
			layout->addWidget(orx2Widget);
	}

	if(layout->count() == 0) {
		widget->hide();
	}

	return widget;
}

QWidget *Adrv9002::createOrxChannelControls(const QString &title, int channel)
{
	// Find the corresponding RX channel (ORX uses same channel as RX but different attributes)
	QString channelName = QString("voltage%1").arg(channel);
	iio_channel *rxCh = iio_device_find_channel(m_iio_dev, channelName.toLocal8Bit().data(), false);

	if(!rxCh) {
		qDebug(CAT_ADRV9002) << "Channel not found:" << channelName;
		return nullptr;
	}

	double dummy;
	int ret = iio_channel_attr_read_double(rxCh, "orx_hardwaregain", &dummy);
	if(ret == -ENODEV) {
		qDebug(CAT_ADRV9002) << "ORX hardware not available on channel" << channel
				     << "- orx_hardwaregain returned -ENODEV";
		return nullptr; // Don't create widget if ORX hardware doesn't exist
	}

	QWidget *widget = new QWidget();
	// Add individual channel styling for borders - matching RX/TX pattern
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QGridLayout *layout = new QGridLayout(widget);
	layout->setSpacing(10);
	layout->setContentsMargins(15, 15, 15, 15);

	// Channel title spanning full width
	QLabel *titleLabel = new QLabel(title);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel, 0, 0, 1, 2); // Span 2 columns

	// Row 1: Hardware Gain (Left) and BBDC Rejection (Right)
	layout->addWidget(createRangeWidget(rxCh, "orx_hardwaregain", "[4 1 36]", "Hardware Gain (dB)"), 1, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "orx_bbdc_rejection_en", "BBDC Rejection"), 1, 1);

	// Row 2: Tracking (Left) and Powerdown (Right)
	layout->addWidget(createCheckboxWidget(rxCh, "orx_quadrature_w_poly_tracking_en", "Quadrature Poly"), 2, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "orx_en", "Powerdown"), 2, 1);

	return widget;
}

// Widget Creation Helper Functions

IIOWidget *Adrv9002::createComboWidget(iio_channel *ch, const QString &attr, const QString &availableAttr,
				       const QString &title)
{
	IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
				    .device(m_iio_dev)
				    .channel(ch)
				    .attribute(attr)
				    .optionsAttribute(availableAttr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi)
				    .buildSingle();

	if(widget) {
		connect(this, &Adrv9002::readRequested, widget, &IIOWidget::readAsync);
	}
	return widget;
}

IIOWidget *Adrv9002::createRangeWidget(iio_channel *ch, const QString &attr, const QString &range, const QString &title)
{
	IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
				    .device(m_iio_dev)
				    .channel(ch)
				    .attribute(attr)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .buildSingle();

	if(widget) {
		// Apply stripUnits to ALL range widgets - safe for all values
		widget->setDataToUIConversion([](QString data) {
			return data.split(" ").first(); // Safe for all numeric attributes
		});

		connect(this, &Adrv9002::readRequested, widget, &IIOWidget::readAsync);
	}
	return widget;
}

IIOWidget *Adrv9002::createCheckboxWidget(iio_channel *ch, const QString &attr, const QString &label)
{
	IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
				    .device(m_iio_dev)
				    .channel(ch)
				    .attribute(attr)
				    .title(label)
				    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				    .buildSingle();

	if(widget) {
		connect(this, &Adrv9002::readRequested, widget, &IIOWidget::readAsync);
		widget->showProgressBar(false);
	}
	return widget;
}

IIOWidget *Adrv9002::createReadOnlyWidget(iio_channel *ch, const QString &attr, const QString &title)
{
	IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
				    .device(m_iio_dev)
				    .channel(ch)
				    .attribute(attr)
				    .title(title)
				    .compactMode(true)
				    .buildSingle();

	if(widget) {
		widget->setEnabled(false);
		widget->showProgressBar(false);

		connect(this, &Adrv9002::readRequested, widget, &IIOWidget::readAsync);
	}
	return widget;
}

IIOWidget *Adrv9002::createContinuousReadOnlyWidget(iio_channel *ch, const QString &attr, const QString &title)
{
	IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
				    .device(m_iio_dev)
				    .channel(ch)
				    .attribute(attr)
				    .title(title)
				    .compactMode(true)
				    .buildSingle();

	if(widget) {
		widget->setEnabled(false);
		widget->showProgressBar(false);

		connect(this, &Adrv9002::readRequested, widget, &IIOWidget::readAsync);
		QTimer *timer = new QTimer(widget);

		QObject::connect(timer, &QTimer::timeout, [widget]() { widget->readAsync(); });
		timer->start(10000);
	}

	return widget;
}
