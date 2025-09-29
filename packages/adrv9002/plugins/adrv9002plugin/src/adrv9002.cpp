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

#include <QLoggingCategory>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
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
	if (InitialCalibrationsWidget::isSupported(m_iio_dev)) {
		m_initialCalibrationsWidget = new InitialCalibrationsWidget(m_iio_dev, this);
	}

	// Create tab system
	m_tabCentralWidget = new QStackedWidget(this);

	// Create the three tab widgets
	m_controlsWidget = createControlsWidget();
	m_blockDiagramWidget = createBlockDiagramWidget();
	m_profileGeneratorWidget = createProfileGeneratorWidget();

	// Add widgets to stack
	m_tabCentralWidget->addWidget(m_controlsWidget);
	m_tabCentralWidget->addWidget(m_blockDiagramWidget);
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

				// Refresh ProfileManager device info when ADRV9002 refreshes
				if (m_profileManager) {
					m_profileManager->updateDeviceInfo();
				}

				// Refresh Initial Calibrations widget when ADRV9002 refreshes
				if (m_initialCalibrationsWidget) {
					m_initialCalibrationsWidget->refreshStatus();
				}

				watcher->deleteLater();
			},
			Qt::QueuedConnection);

		auto future = QtConcurrent::run([this]() { Q_EMIT readRequested(); });
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
	m_blockDiagramBtn = new QPushButton("Block Diagram", this);
	m_profileGeneratorBtn = new QPushButton("Profile Generator", this);

	// Make buttons checkable and style them
	m_controlsBtn->setCheckable(true);
	m_blockDiagramBtn->setCheckable(true);
	m_profileGeneratorBtn->setCheckable(true);

	Style::setStyle(m_controlsBtn, style::properties::button::blueGrayButton);
	Style::setStyle(m_blockDiagramBtn, style::properties::button::blueGrayButton);
	Style::setStyle(m_profileGeneratorBtn, style::properties::button::blueGrayButton);

	// Connect tab switching
	connect(m_controlsBtn, &QPushButton::clicked, this,
		[=, this]() { m_tabCentralWidget->setCurrentWidget(m_controlsWidget); });
	connect(m_blockDiagramBtn, &QPushButton::clicked, this,
		[=, this]() { m_tabCentralWidget->setCurrentWidget(m_blockDiagramWidget); });
	connect(m_profileGeneratorBtn, &QPushButton::clicked, this,
		[=, this]() { m_tabCentralWidget->setCurrentWidget(m_profileGeneratorWidget); });

	// Add to button group
	m_tabButtons->addButton(m_controlsBtn);
	m_tabButtons->addButton(m_blockDiagramBtn);
	m_tabButtons->addButton(m_profileGeneratorBtn);

	// Add buttons to top container
	m_tool->addWidgetToTopContainerHelper(m_controlsBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_blockDiagramBtn, TTA_LEFT);
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
		// Create section widgets following ad936x pattern
		mainLayout->addWidget(generateGlobalSettingsWidget(centralWidget));
		mainLayout->addWidget(generateReceiveChainWidget(centralWidget));
		mainLayout->addWidget(generateTransmitChainWidget(centralWidget));
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

QWidget *Adrv9002::createBlockDiagramWidget()
{
	QWidget *widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(widget);

	QLabel *placeholder = new QLabel("Block Diagram - Coming Soon", widget);
	placeholder->setAlignment(Qt::AlignCenter);
	Style::setStyle(placeholder, style::properties::label::menuBig);
	layout->addWidget(placeholder);

	return widget;
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

QWidget *Adrv9002::generateGlobalSettingsWidget(QWidget *parent)
{
	QWidget *globalSettingsWidget = new QWidget(parent);
	Style::setBackgroundColor(globalSettingsWidget, json::theme::background_primary);
	Style::setStyle(globalSettingsWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(globalSettingsWidget);
	globalSettingsWidget->setLayout(layout);

	QLabel *title = new QLabel("ADRV9002 Global Settings", globalSettingsWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// Profile Manager - handles profile_config and stream_config
	if(m_profileManager) {
		layout->addWidget(m_profileManager);
	}

	// Temperature (separate from profile management)
	iio_channel *tempCh = iio_device_find_channel(m_iio_dev, "temp0", false);
	if(tempCh) {
		layout->addWidget(createReadOnlyLabel(tempCh, "input", 1000, "Temperature (°C)"));
	}

	return globalSettingsWidget;
}

QWidget *Adrv9002::generateReceiveChainWidget(QWidget *parent)
{
	QWidget *rxChainWidget = new QWidget(parent);
	Style::setBackgroundColor(rxChainWidget, json::theme::background_primary);
	Style::setStyle(rxChainWidget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(rxChainWidget);
	rxChainWidget->setLayout(mainLayout);

	QLabel *title = new QLabel("ADRV9002 Receive Chain", rxChainWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	mainLayout->addWidget(title);

	// Main RX section with RX1 and RX2 side by side
	QHBoxLayout *rxLayout = new QHBoxLayout();
	rxLayout->setSpacing(15);

	// RX1 Column
	rxLayout->addWidget(createRxChannelControls("RX 1", 0));

	// RX2 Column
	rxLayout->addWidget(createRxChannelControls("RX 2", 1));

	mainLayout->addLayout(rxLayout);

	// ORX Section below RX1/RX2 (separate row)
	QHBoxLayout *orxLayout = new QHBoxLayout();
	orxLayout->addWidget(createOrxControls());
	mainLayout->addLayout(orxLayout);

	return rxChainWidget;
}

QWidget *Adrv9002::generateTransmitChainWidget(QWidget *parent)
{
	QWidget *txChainWidget = new QWidget(parent);
	Style::setBackgroundColor(txChainWidget, json::theme::background_primary);
	Style::setStyle(txChainWidget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(txChainWidget);
	txChainWidget->setLayout(mainLayout);

	QLabel *title = new QLabel("ADRV9002 Transmit Chain", txChainWidget);
	Style::setStyle(title, style::properties::label::menuBig);
	mainLayout->addWidget(title);

	QHBoxLayout *txLayout = new QHBoxLayout();
	txLayout->setSpacing(15);

	// TX1 Column
	txLayout->addWidget(createTxChannelControls("TX 1", 0));

	// TX2 Column
	txLayout->addWidget(createTxChannelControls("TX 2", 1));

	mainLayout->addLayout(txLayout);
	return txChainWidget;
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

	// Match exact iio-oscilloscope layout: 2 columns from original columns 2&4
	int row = 1;

	// Column 0 (Left) - from iio-oscilloscope column 2
	layout->addWidget(createRangeWidget(rxCh, "hardwaregain", "[0 0.5 36]", "Hardware Gain (dB)"), row, 0);
	layout->addWidget(createComboWidget(rxCh, "gain_control_mode", "gain_control_mode_available", "Gain Control"),
			  row + 1, 0);
	layout->addWidget(createComboWidget(rxCh, "ensm_mode", "ensm_mode_available", "ENSM"), row + 2, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "en", "Powerdown"), row + 3, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "bbdc_rejection_en", "BBDC Rejection"), row + 4, 0);
	layout->addWidget(createRangeWidget(rxCh, "nco_frequency", "[-20000 1 20000]", "NCO (Hz)"), row + 5, 0);
	layout->addWidget(createReadOnlyLabel(rxCh, "decimated_power", 1.0, "Decimated Power (dB)"), row + 6, 0);
	layout->addWidget(createReadOnlyLabel(rxCh, "rf_bandwidth", 1000000, "Bandwidth (MHz)"), row + 7, 0);

	// Column 1 (Right) - from iio-oscilloscope column 4
	layout->addWidget(createComboWidget(rxCh, "digital_gain_control_mode", "digital_gain_control_mode_available",
					    "Digital Gain Control"),
			  row, 1);
	layout->addWidget(createComboWidget(rxCh, "interface_gain", "interface_gain_available", "Interface Gain (dB)"),
			  row + 1, 1);
	layout->addWidget(createComboWidget(rxCh, "port_en_mode", "port_en_mode_available", "Port Enable"), row + 2, 1);
	layout->addWidget(createCheckboxWidget(rxCh, "dynamic_adc_switch_en", "Dynamic Adc Switch"), row + 3, 1);
	layout->addWidget(createRangeWidget(rxCh, "bbdc_loop_gain", "[-20 1 20]", "BBDC Loop Gain (dB)"), row + 4, 1);

	if(loCh) {
		QString loAttr = QString("RX%1_LO_frequency").arg(channel + 1);
		layout->addWidget(createRangeWidget(loCh, loAttr, "[30 1 6000]", "Local Oscillator (MHz)"), row + 5, 1);
	}

	layout->addWidget(createReadOnlyLabel(rxCh, "rssi", 1.0, "RSSI (dB)"), row + 6, 1);
	layout->addWidget(createReadOnlyLabel(rxCh, "sampling_frequency", 1000000, "Sampling Rate (MSPS)"), row + 7, 1);

	// Tracking section at bottom spanning both columns (like iio-oscilloscope)
	int trackingStartRow = row + 9;
	QLabel *trackingLabel = new QLabel("Tracking:");
	Style::setStyle(trackingLabel, style::properties::label::menuBig);
	layout->addWidget(trackingLabel, trackingStartRow, 0, 1, 2);

	// Create tracking checkboxes in 2x3 grid layout matching iio-oscilloscope
	layout->addWidget(createCheckboxWidget(rxCh, "quadrature_fic_tracking_en", "Quadrature FIC"),
			  trackingStartRow + 1, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "agc_tracking_en", "AGC"), trackingStartRow + 1, 1);
	layout->addWidget(createCheckboxWidget(rxCh, "bbdc_rejection_tracking_en", "BBDC Rejection"),
			  trackingStartRow + 2, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "quadrature_w_poly_tracking_en", "Quadrature Poly"),
			  trackingStartRow + 2, 1);
	layout->addWidget(createCheckboxWidget(rxCh, "hd_tracking_en", "HD2"), trackingStartRow + 3, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "rssi_tracking_en", "RSSI"), trackingStartRow + 3, 1);

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

	// Match exact iio-oscilloscope TX layout: 2 columns from original columns 2&4
	int row = 1;

	// Column 0 (Left) - from iio-oscilloscope TX column 2
	layout->addWidget(createRangeWidget(txCh, "hardwaregain", "[-41.95 0.05 0]", "Attenuation (dB)"), row, 0);
	layout->addWidget(
		createComboWidget(txCh, "atten_control_mode", "atten_control_mode_available", "Attenuation Control"),
		row + 1, 0);

	if(loCh) {
		QString loAttr = QString("TX%1_LO_frequency").arg(channel + 1);
		layout->addWidget(createRangeWidget(loCh, loAttr, "[30 1 6000]", "Local Oscillator (MHz)"), row + 2, 0);
	}

	layout->addWidget(createRangeWidget(txCh, "nco_frequency", "[-20000 1 20000]", "NCO (Hz)"), row + 3, 0);
	layout->addWidget(createReadOnlyLabel(txCh, "rf_bandwidth", 1000000, "Bandwidth (MHz)"), row + 4, 0);

	// Column 1 (Right) - from iio-oscilloscope TX column 4
	layout->addWidget(createComboWidget(txCh, "port_en_mode", "port_en_mode_available", "Port Enable"), row, 1);
	layout->addWidget(createComboWidget(txCh, "ensm_mode", "ensm_mode_available", "ENSM"), row + 1, 1);
	layout->addWidget(createCheckboxWidget(txCh, "en", "Powerdown"), row + 2, 1);
	// Row+3 is empty in iio-oscilloscope
	layout->addWidget(createReadOnlyLabel(txCh, "sampling_frequency", 1000000, "Sampling Rate (MSPS)"), row + 4, 1);

	// Tracking section at bottom spanning both columns (like iio-oscilloscope)
	int trackingStartRow = row + 6;
	QLabel *trackingLabel = new QLabel("Tracking:");
	Style::setStyle(trackingLabel, style::properties::label::menuBig);
	layout->addWidget(trackingLabel, trackingStartRow, 0, 1, 2);

	// Create tracking checkboxes in 2x3 grid layout matching iio-oscilloscope
	layout->addWidget(createCheckboxWidget(txCh, "quadrature_tracking_en", "Quadrature"), trackingStartRow + 1, 0);
	layout->addWidget(createCheckboxWidget(txCh, "close_loop_gain_tracking_en", "Close Loop Gain"),
			  trackingStartRow + 1, 1);
	layout->addWidget(createCheckboxWidget(txCh, "pa_correction_tracking_en", "PA Correction"),
			  trackingStartRow + 2, 0);
	layout->addWidget(createCheckboxWidget(txCh, "loopback_delay_tracking_en", "Loopback Delay"),
			  trackingStartRow + 2, 1);
	layout->addWidget(createCheckboxWidget(txCh, "lo_leakage_tracking_en", "LO Leakage"), trackingStartRow + 3, 0);

	return widget;
}

QWidget *Adrv9002::createOrxControls()
{
	QWidget *widget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout(widget);
	layout->setSpacing(15);

	// ORX 1 Column
	layout->addWidget(createOrxChannelControls("ORX 1", 0));

	// ORX 2 Column
	layout->addWidget(createOrxChannelControls("ORX 2", 1));

	return widget;
}

QWidget *Adrv9002::createOrxChannelControls(const QString &title, int channel)
{
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

	// Find the corresponding RX channel (ORX uses same channel as RX but different attributes)
	QString channelName = QString("voltage%1").arg(channel);
	iio_channel *rxCh = iio_device_find_channel(m_iio_dev, channelName.toLocal8Bit().data(), false);

	if(!rxCh) {
		QLabel *errorLabel = new QLabel("Channel not found");
		layout->addWidget(errorLabel, 1, 0, 1, 2);
		return widget;
	}

	// Match exact iio-oscilloscope ORX layout from screenshots: 2x2 grid
	int row = 1;

	// Row 1: Hardware Gain (Left) and BBDC Rejection (Right)
	layout->addWidget(createRangeWidget(rxCh, "orx_hardwaregain", "[4 1 36]", "Hardware Gain (dB)"), row, 0);
	layout->addWidget(createCheckboxWidget(rxCh, "orx_bbdc_rejection_en", "BBDC Rejection"), row, 1);

	// Row 2: Tracking (Left) and Powerdown (Right)
	layout->addWidget(createCheckboxWidget(rxCh, "orx_quadrature_w_poly_tracking_en", "Quadrature Poly"), row + 1,
			  0);
	layout->addWidget(createCheckboxWidget(rxCh, "orx_en", "Powerdown"), row + 1, 1);

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
	}
	return widget;
}

QLabel *Adrv9002::createReadOnlyLabel(iio_channel *ch, const QString &attr, double divisor, const QString &title)
{
	QLabel *label = new QLabel(title + ": N/A");

	if(!ch) {
		return label;
	}

	// Read the attribute value
	char value[64];
	int ret = iio_channel_attr_read(ch, attr.toLocal8Bit().data(), value, sizeof(value));

	if(ret > 0) {
		// Parse the value and apply divisor
		bool ok;
		double numValue = QString(value).toDouble(&ok);
		if(ok) {
			// Apply divisor and format to appropriate precision
			double displayValue = numValue / divisor;

			// Format based on the magnitude - temperature typically 4 decimal places
			if(title.contains("Temperature", Qt::CaseInsensitive)) {
				label->setText(title + ": " + QString::number(displayValue, 'f', 4));
			} else if(divisor == 1000000.0) { // MHz values
				label->setText(title + ": " + QString::number(displayValue, 'f', 3));
			} else {
				label->setText(title + ": " + QString::number(displayValue, 'f', 2));
			}
		} else {
			label->setText(title + ": " + QString(value)); // Show as-is if not numeric
		}
	} else {
		label->setText(title + ": Error");
	}

	// Connect to refresh signal for periodic updates
	connect(this, &Adrv9002::readRequested, this, [this, label, ch, attr, divisor, title]() {
		char value[64];
		int ret = iio_channel_attr_read(ch, attr.toLocal8Bit().data(), value, sizeof(value));

		if(ret > 0) {
			bool ok;
			double numValue = QString(value).toDouble(&ok);
			if(ok) {
				double displayValue = numValue / divisor;

				if(title.contains("Temperature", Qt::CaseInsensitive)) {
					label->setText(title + ": " + QString::number(displayValue, 'f', 4));
				} else if(divisor == 1000000.0) {
					label->setText(title + ": " + QString::number(displayValue, 'f', 3));
				} else {
					label->setText(title + ": " + QString::number(displayValue, 'f', 2));
				}
			} else {
				label->setText(title + ": " + QString(value));
			}
		} else {
			label->setText(title + ": Error");
		}
	});

	return label;
}
