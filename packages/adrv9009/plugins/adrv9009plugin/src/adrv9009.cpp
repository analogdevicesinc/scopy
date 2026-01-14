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

#include "adrv9009.h"
#include <QLabel>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QGridLayout>
#include <QFormLayout>
#include <cmath>
#include <gui/widgets/menusectionwidget.h>
#include <style.h>
#include <QFutureWatcher>
#include <QLoggingCategory>
#include <QTimer>
#include <qtconcurrentrun.h>
#include <filebrowserwidget.h>
#include <pkg-manager/pkgmanager.h>

Q_LOGGING_CATEGORY(CAT_ADRV9009, "ADRV9009");

using namespace scopy;
using namespace scopy::adrv9009;

Adrv9009::Adrv9009(iio_context *ctx, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_tool(nullptr)
	, m_refreshButton(nullptr)
	, m_centralWidget(nullptr)
{
	setupUi();
	qDebug(CAT_ADRV9009) << "ADRV9009 tool initialized successfully";
}

Adrv9009::~Adrv9009() {}

void Adrv9009::setupUi()
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

	m_centralWidget = new QWidget(this);

	QVBoxLayout *centralWidgetLayout = new QVBoxLayout(m_centralWidget);
	centralWidgetLayout->setMargin(0);
	centralWidgetLayout->setContentsMargins(0, 0, 0, 0);
	m_centralWidget->setLayout(centralWidgetLayout);

	QWidget *controlsScrollWidget = new QWidget(this);
	QVBoxLayout *controlsScrollLayout = new QVBoxLayout(controlsScrollWidget);
	controlsScrollLayout->setMargin(0);
	controlsScrollLayout->setContentsMargins(0, 0, 0, 0);
	controlsScrollWidget->setLayout(controlsScrollLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsScrollWidget);

	centralWidgetLayout->addWidget(scrollArea);

	if(!m_ctx) {
		qWarning(CAT_ADRV9009) << "No context provided";
		return;
	}

	detectAndStoreDevices();

	if(!m_adrv9009DeviceMap.isEmpty()) {
		controlsScrollLayout->addWidget(
			generateGlobalSettingsWidget("ADRV9009 Global Settings", controlsScrollWidget));
		controlsScrollLayout->addWidget(generateRxChainWidget("ADRV9009 Receive Chain", controlsScrollWidget));
		controlsScrollLayout->addWidget(generateTxChainWidget("ADRV9009 Transmit Chain", controlsScrollWidget));
		controlsScrollLayout->addWidget(
			generateObsRxChainWidget("ADRV9009 Observation RX", controlsScrollWidget));
		controlsScrollLayout->addWidget(generateFpgaSettingsWidget("FPGA Settings", controlsScrollWidget));

		controlsScrollLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

		// Add MCS Sync button for multi-device mode (exact iio-osc behavior)
		if(m_multiDeviceMode) {
			m_tool->bottomContainer()->setVisible(true);

			m_mcsButton = new QPushButton("MCS Sync", this);
			Style::setStyle(m_mcsButton, style::properties::button::basicButton);
			connect(m_mcsButton, &QPushButton::clicked, this, &Adrv9009::performMcsSync);
			m_tool->addWidgetToBottomContainerHelper(m_mcsButton, TTA_LEFT);

			qDebug(CAT_ADRV9009) << "MCS Sync button added for" << m_adrv9009DeviceMap.size() << "devices";

			// Auto-sync after initialization (like iio-osc)
			QTimer::singleShot(100, this, [this]() { performMcsSync(); });
		}
	} else {
		qWarning(CAT_ADRV9009) << "No ADRV9009 devices found in context";
		return;
	}

	m_tool->addWidgetToCentralContainerHelper(m_centralWidget);
}

void Adrv9009::detectAndStoreDevices()
{
	m_adrv9009DeviceMap.clear();

	if(!m_ctx) {
		qWarning(CAT_ADRV9009) << "No context provided";
		return;
	}

	unsigned int deviceCount = iio_context_get_devices_count(m_ctx);

	for(unsigned int i = 0; i < deviceCount; i++) {
		iio_device *dev = iio_context_get_device(m_ctx, i);
		const char *deviceName = iio_device_get_name(dev);

		if(deviceName && QString(deviceName).startsWith("adrv9009-phy")) {
			m_adrv9009DeviceMap[QString(deviceName)] = dev;
			qDebug(CAT_ADRV9009) << "Found ADRV9009 device:" << deviceName;
		}
	}

	m_multiDeviceMode = (m_adrv9009DeviceMap.size() > 1);
	qDebug(CAT_ADRV9009) << "Total ADRV9009 devices found:" << m_adrv9009DeviceMap.size();
	qDebug(CAT_ADRV9009) << "Multi-device mode:" << m_multiDeviceMode;
}

void Adrv9009::performMcsSync()
{
	// 1. Skip if single device (exact iio-osc check)
	if(!m_multiDeviceMode) {
		return;
	}

	qDebug(CAT_ADRV9009) << "Starting MCS sync for" << m_adrv9009DeviceMap.size() << "devices";

	// 2. Try JESD204-FSM automatic sync (exact magic number from iio-osc!)
	QStringList deviceNames = m_adrv9009DeviceMap.keys();
	iio_device *firstDevice = m_adrv9009DeviceMap[deviceNames.first()];
	int ret = iio_device_attr_write_longlong(firstDevice, "multichip_sync", multichipSyncValue);
	if(ret != -EINVAL) {
		qDebug(CAT_ADRV9009) << "JESD204-FSM automatic sync successful";
		Q_EMIT readRequested(); // Refresh all widgets
		return;
	}

	qDebug(CAT_ADRV9009) << "JESD204-FSM not available, using manual sync";

	// 3. Configure HMC7044 clock distributor (exact iio-osc logic)
	iio_device *hmc7044_dev = iio_context_find_device(m_ctx, "hmc7044");
	if(hmc7044_dev) {
		unsigned int val;
		int ret = iio_device_reg_read(hmc7044_dev, 0x5a, &val);
		// Is continuous mode?
		if(!ret && val == 7) {
			iio_device_reg_write(hmc7044_dev, 0x5a, 0);
			qDebug(CAT_ADRV9009) << "HMC7044 REG 0x5A set to level sensitive GPI SYSREF request";
		}
	}

	// 4. Manual sync sequence for all devices (exact iio-osc sequence)
	for(int i = 0; i <= 11; i++) {
		for(const QString &deviceName : deviceNames) {
			iio_device *device = m_adrv9009DeviceMap[deviceName];
			iio_device_attr_write_longlong(device, "multichip_sync", i);
		}
	}

	qDebug(CAT_ADRV9009) << "Manual MCS sync sequence completed";
	Q_EMIT readRequested(); // Refresh all widgets
}

QWidget *Adrv9009::generateGlobalSettingsWidget(QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *globalSection = new MenuSectionCollapseWidget(
		title, MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	if(!m_multiDeviceMode) {
		// Single device mode - use first (and only) device
		iio_device *device = m_adrv9009DeviceMap.first();
		QWidget *content = createGlobalSettingsContentForDevice(device, globalSection);
		globalSection->contentLayout()->addWidget(content);
	} else {
		// Multi-device mode - create tabs
		QTabWidget *deviceTabs = new QTabWidget(globalSection);

		QStringList deviceNames = m_adrv9009DeviceMap.keys();
		for(int i = 0; i < deviceNames.size(); i++) {
			QString deviceName = deviceNames[i];
			iio_device *device = m_adrv9009DeviceMap[deviceName];

			QWidget *deviceContent = createGlobalSettingsContentForDevice(device, deviceTabs);
			deviceTabs->addTab(deviceContent, deviceName);
		}

		globalSection->contentLayout()->addWidget(deviceTabs);
	}

	return globalSection;
}

QWidget *Adrv9009::createGlobalSettingsContentForDevice(iio_device *dev, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	if(dev == nullptr) {
		qWarning(CAT_ADRV9009) << "No ADRV9009 device found";
		return widget;
	}

	QGridLayout *layout = new QGridLayout();

	// Active ENSM and ENSM Modes are one widget
	IIOWidget *ensmWidget = IIOWidgetBuilder(widget)
					.device(dev)
					.attribute("ensm_mode")
					.optionsAttribute("ensm_mode_available")
					.title("ENSM mode")
					.uiStrategy(IIOWidgetBuilder::ComboUi)
					.buildSingle();
	layout->addWidget(ensmWidget, 1, 0);
	connect(this, &Adrv9009::readRequested, ensmWidget, &IIOWidget::readAsync);

	// Load Profile section
	QLabel *loadProflieLabel = new QLabel("Load Profile");
	QFileInfoList filters = PkgManager::listFilesInfo(QStringList() << "adrv9009-filters");
	QString defaultDir = (filters.size() > 0) ? filters.first().absolutePath() : PkgManager::packagesPath();

	FileBrowserWidget *profileWidget = new FileBrowserWidget(FileBrowserWidget::OPEN_FILE, widget);
	profileWidget->setFilter("Profile files (*.txt);;All files (*.*)");
	profileWidget->setBaseDirectory(defaultDir);
	layout->addWidget(loadProflieLabel, 0, 1);
	layout->addWidget(profileWidget, 1, 1);

	connect(profileWidget->lineEdit(), &QLineEdit::textChanged, this, &Adrv9009::loadProfileFromFile);

	// TRX Local Oscillator framed widget
	QWidget *trxLoWidget = new QWidget(widget);
	Style::setBackgroundColor(trxLoWidget, json::theme::background_primary);
	Style::setStyle(trxLoWidget, style::properties::widget::border_interactive);
	QVBoxLayout *trxLoLayout = new QVBoxLayout(trxLoWidget);

	QLabel *trxLoTitle = new QLabel("TRX Local Oscillator", trxLoWidget);
	Style::setStyle(trxLoTitle, style::properties::label::menuMedium);
	trxLoLayout->addWidget(trxLoTitle);

	iio_channel *trxLo = iio_device_find_channel(dev, "altvoltage0", true); // TRX_LO
	if(trxLo) {
		// Frequency(MHz)
		IIOWidget *trxLoFreq =
			Adrv9009WidgetFactory::createRangeWidget(trxLo, "frequency", "[70 1 6000]", "Frequency(MHz)");
		connect(this, &Adrv9009::readRequested, trxLoFreq, &IIOWidget::readAsync);
		trxLoFreq->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		trxLoFreq->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });

		trxLoLayout->addWidget(trxLoFreq);

		// Frequency Hopping Mode
		IIOWidget *fhm = Adrv9009WidgetFactory::createCheckboxWidget(trxLo, "frequency_hopping_mode_enable",
									     "Frequency Hopping Mode");
		connect(this, &Adrv9009::readRequested, fhm, &IIOWidget::readAsync);
		trxLoLayout->addWidget(fhm);
	}

	layout->addWidget(trxLoWidget, 2, 0);

	layout->addWidget(generateCalibrationWidget(dev, widget), 2, 1);

	mainLayout->addLayout(layout);

	qDebug(CAT_ADRV9009) << "Global Settings widgets created successfully";
	return widget;
}

void Adrv9009::loadProfileFromFile(QString filePath)
{
	if(!filePath.isEmpty()) {
		QFile file(filePath);
		if(!file.open(QIODevice::ReadOnly)) {
			qWarning(CAT_ADRV9009) << "Failed to open profile:" << file.errorString();
			return;
		}

		QByteArray buffer = file.readAll();
		file.close();

		// Write profile to device (use first device)
		iio_device *device = m_adrv9009DeviceMap.first();
		int ret = iio_device_attr_write_raw(device, "profile_config", buffer.constData(), buffer.size());

		if(ret < 0) {
			qWarning(CAT_ADRV9009) << "Profile loading failed, error:" << ret;
		} else {
			qDebug(CAT_ADRV9009) << "Profile loaded successfully";
			Q_EMIT readRequested(); // Refresh all widgets
		}

	} else {
		qWarning(CAT_ADRV9009) << "Profile loading failed, no file path provided";
	}
}

QWidget *Adrv9009::generateCalibrationWidget(iio_device *device, QWidget *parent)
{
	QWidget *calibrationsWidget = new QWidget(parent);
	Style::setBackgroundColor(calibrationsWidget, json::theme::background_primary);
	Style::setStyle(calibrationsWidget, style::properties::widget::border_interactive);
	QVBoxLayout *calibrationsLayout = new QVBoxLayout(calibrationsWidget);

	QLabel *calibrationsTitle = new QLabel("Calibrations", calibrationsWidget);
	Style::setStyle(calibrationsTitle, style::properties::label::menuMedium);
	calibrationsLayout->addWidget(calibrationsTitle);

	// Calibration checkboxes in grid layout (like iio-osc)
	QGridLayout *calGridLayout = new QGridLayout();

	// Row 1 checkboxes
	IIOWidget *calRxQec = Adrv9009WidgetFactory::createCheckboxWidget(device, "calibrate_rx_qec_en", "CAL RX QEC");
	connect(this, &Adrv9009::readRequested, calRxQec, &IIOWidget::readAsync);
	IIOWidget *calTxQec = Adrv9009WidgetFactory::createCheckboxWidget(device, "calibrate_tx_qec_en", "CAL TX QEC");
	connect(this, &Adrv9009::readRequested, calTxQec, &IIOWidget::readAsync);
	IIOWidget *calTxLol = Adrv9009WidgetFactory::createCheckboxWidget(device, "calibrate_tx_lol_en", "CAL TX LOL");
	connect(this, &Adrv9009::readRequested, calTxLol, &IIOWidget::readAsync);
	IIOWidget *calTxLolExt =
		Adrv9009WidgetFactory::createCheckboxWidget(device, "calibrate_tx_lol_ext_en", "CAL TX LOL Ext.");
	connect(this, &Adrv9009::readRequested, calTxLolExt, &IIOWidget::readAsync);

	calGridLayout->addWidget(calRxQec, 0, 0);
	calGridLayout->addWidget(calTxQec, 0, 1);
	calGridLayout->addWidget(calTxLol, 0, 2);
	calGridLayout->addWidget(calTxLolExt, 0, 3);

	// Row 2 checkboxes
	IIOWidget *calRxPhaseCorr = Adrv9009WidgetFactory::createCheckboxWidget(
		device, "calibrate_rx_phase_correction_en", "CAL RX PHASE CORR");
	connect(this, &Adrv9009::readRequested, calRxPhaseCorr, &IIOWidget::readAsync);
	IIOWidget *calFhm = Adrv9009WidgetFactory::createCheckboxWidget(device, "calibrate_fhm_en", "CAL FHM");
	connect(this, &Adrv9009::readRequested, calFhm, &IIOWidget::readAsync);

	calGridLayout->addWidget(calRxPhaseCorr, 1, 0);
	calGridLayout->addWidget(calFhm, 1, 1);

	// CALIBRATE button
	QPushButton *calibrateButton = new QPushButton("CALIBRATE", calibrationsWidget);
	Style::setStyle(calibrateButton, style::properties::button::basicButton);
	calGridLayout->addWidget(calibrateButton, 1, 3);

	connect(calibrateButton, &QPushButton::clicked, this, [=] {
		// Trigger calibration
		int ret = iio_device_attr_write_bool(device, "calibrate", true);
		if(ret < 0) {
			qWarning(CAT_ADRV9009) << "Calibration failed:" << ret;
		} else {
			qDebug(CAT_ADRV9009) << "Calibration triggered";
			Q_EMIT readRequested(); // Refresh widgets
		}
	});

	calibrationsLayout->addLayout(calGridLayout);

	return calibrationsWidget;
}

QWidget *Adrv9009::generateRxChainWidget(QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *rxSection = new MenuSectionCollapseWidget(
		title, MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	if(!m_multiDeviceMode) {
		// Single device mode - use first (and only) device
		iio_device *device = m_adrv9009DeviceMap.first();
		QWidget *content = createRxChainContentForDevice(device, rxSection);
		rxSection->contentLayout()->addWidget(content);
	} else {
		// Multi-device mode - create tabs
		QTabWidget *deviceTabs = new QTabWidget(rxSection);

		QStringList deviceNames = m_adrv9009DeviceMap.keys();
		for(int i = 0; i < deviceNames.size(); i++) {
			QString deviceName = deviceNames[i];
			iio_device *device = m_adrv9009DeviceMap[deviceName];

			QWidget *deviceContent = createRxChainContentForDevice(device, deviceTabs);
			deviceTabs->addTab(deviceContent, deviceName);
		}

		rxSection->contentLayout()->addWidget(deviceTabs);
	}

	return rxSection;
}

QWidget *Adrv9009::createRxChainContentForDevice(iio_device *dev, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	if(dev == nullptr) {
		qWarning(CAT_ADRV9009) << "No ADRV9009 device found for RX chain";
		return widget;
	}

	// Section-level controls
	QHBoxLayout *sectionControlsLayout = new QHBoxLayout();

	iio_channel *rxChannel0 = iio_device_find_channel(dev, "voltage0", false);

	if(rxChannel0) {
		// RF Bandwidth (read-only, shared for RX section)
		IIOWidget *rfBandwidthWidget = Adrv9009WidgetFactory::createReadOnlyWidget(rxChannel0, "rf_bandwidth",
											   "RF Bandwidth(MHz)", false);
		connect(this, &Adrv9009::readRequested, rfBandwidthWidget, &IIOWidget::readAsync);
		rfBandwidthWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(rfBandwidthWidget);

		// Sampling Rate (read-only, shared for RX section)
		IIOWidget *samplingRateWidget = Adrv9009WidgetFactory::createReadOnlyWidget(
			rxChannel0, "sampling_frequency", "Sampling Rate(MSPS)", false);
		connect(this, &Adrv9009::readRequested, samplingRateWidget, &IIOWidget::readAsync);
		samplingRateWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(samplingRateWidget);

		// Gain Control Modes (shared dropdown for RX section)
		IIOWidget *gainControlModes = Adrv9009WidgetFactory::createComboWidget(
			rxChannel0, "gain_control_mode", "gain_control_mode_available", "Gain Control Modes");
		connect(this, &Adrv9009::readRequested, gainControlModes, &IIOWidget::readAsync);
		sectionControlsLayout->addWidget(gainControlModes);
	}

	mainLayout->addLayout(sectionControlsLayout);

	// Side-by-side RX1/RX2 layout
	QHBoxLayout *rxLayout = new QHBoxLayout();

	// Create RX1 and RX2 channel widgets
	QWidget *rx1Widget = createRxChannelWidget(dev, "RX1", 0, widget);
	QWidget *rx2Widget = createRxChannelWidget(dev, "RX2", 1, widget);

	rxLayout->addWidget(rx1Widget);
	rxLayout->addWidget(rx2Widget);

	mainLayout->addLayout(rxLayout);

	qDebug(CAT_ADRV9009) << "RX Chain widgets created successfully";
	return widget;
}

QWidget *Adrv9009::generateTxChainWidget(QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *txSection = new MenuSectionCollapseWidget(
		title, MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	if(!m_multiDeviceMode) {
		// Single device mode - use first (and only) device
		iio_device *device = m_adrv9009DeviceMap.first();
		QWidget *content = createTxChainContentForDevice(device, txSection);
		txSection->contentLayout()->addWidget(content);
	} else {
		// Multi-device mode - create tabs
		QTabWidget *deviceTabs = new QTabWidget(txSection);

		QStringList deviceNames = m_adrv9009DeviceMap.keys();
		for(int i = 0; i < deviceNames.size(); i++) {
			QString deviceName = deviceNames[i];
			iio_device *device = m_adrv9009DeviceMap[deviceName];

			QWidget *deviceContent = createTxChainContentForDevice(device, deviceTabs);
			deviceTabs->addTab(deviceContent, deviceName);
		}

		txSection->contentLayout()->addWidget(deviceTabs);
	}

	return txSection;
}

QWidget *Adrv9009::createTxChainContentForDevice(iio_device *dev, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	if(dev == nullptr) {
		qWarning(CAT_ADRV9009) << "No ADRV9009 device found for TX chain";
		return widget;
	}

	// Section-level controls (like iio-osc)
	QHBoxLayout *sectionControlsLayout = new QHBoxLayout();

	// Find TX output channel for chain-level controls
	iio_channel *txChannel0 = iio_device_find_channel(dev, "voltage0", true);

	if(txChannel0) {
		// RF Bandwidth (read-only, shared for TX section)
		IIOWidget *rfBandwidthWidget = Adrv9009WidgetFactory::createReadOnlyWidget(txChannel0, "rf_bandwidth",
											   "RF Bandwidth(MHz)", false);
		rfBandwidthWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(rfBandwidthWidget);

		// Sampling Rate (read-only, shared for TX section)
		IIOWidget *samplingRateWidget = Adrv9009WidgetFactory::createReadOnlyWidget(
			txChannel0, "sampling_frequency", "Sampling Rate(MSPS)", false);
		samplingRateWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(samplingRateWidget);

		// PA Protection (shared checkbox for TX section)
		IIOWidget *paProtection =
			Adrv9009WidgetFactory::createCheckboxWidget(txChannel0, "pa_protection_en", "PA Protection");
		connect(this, &Adrv9009::readRequested, paProtection, &IIOWidget::readAsync);
		sectionControlsLayout->addWidget(paProtection);
	}
	mainLayout->addLayout(sectionControlsLayout);

	// Side-by-side TX1/TX2 layout
	QHBoxLayout *txLayout = new QHBoxLayout();

	// Create TX1 and TX2 channel widgets
	QWidget *tx1Widget = createTxChannelWidget(dev, "TX1", 0, widget);
	QWidget *tx2Widget = createTxChannelWidget(dev, "TX2", 1, widget);

	txLayout->addWidget(tx1Widget);
	txLayout->addWidget(tx2Widget);

	mainLayout->addLayout(txLayout);

	qDebug(CAT_ADRV9009) << "TX Chain widgets created successfully";
	return widget;
}

QWidget *Adrv9009::generateObsRxChainWidget(QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *obsSection = new MenuSectionCollapseWidget(
		title, MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	if(!m_multiDeviceMode) {
		// Single device mode - use first (and only) device
		iio_device *device = m_adrv9009DeviceMap.first();
		QWidget *content = createObsRxChainContentForDevice(device, obsSection);
		obsSection->contentLayout()->addWidget(content);
	} else {
		// Multi-device mode - create tabs
		QTabWidget *deviceTabs = new QTabWidget(obsSection);

		QStringList deviceNames = m_adrv9009DeviceMap.keys();
		for(int i = 0; i < deviceNames.size(); i++) {
			QString deviceName = deviceNames[i];
			iio_device *device = m_adrv9009DeviceMap[deviceName];

			QWidget *deviceContent = createObsRxChainContentForDevice(device, deviceTabs);
			deviceTabs->addTab(deviceContent, deviceName);
		}

		obsSection->contentLayout()->addWidget(deviceTabs);
	}

	return obsSection;
}

QWidget *Adrv9009::createObsRxChainContentForDevice(iio_device *dev, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	if(dev == nullptr) {
		qWarning(CAT_ADRV9009) << "No ADRV9009 device found for OBS RX chain";
		return widget;
	}

	// Section-level controls
	QHBoxLayout *sectionControlsLayout = new QHBoxLayout();

	// Find channels for chain-level controls
	iio_channel *obsChannel0 = iio_device_find_channel(dev, "voltage2", false); // For RF/sampling
	iio_channel *auxLo = iio_device_find_channel(dev, "altvoltage1", true);	    // For AUX LO

	if(obsChannel0) {
		// RF Bandwidth (read-only, shared for OBS section)
		IIOWidget *rfBandwidthWidget = Adrv9009WidgetFactory::createReadOnlyWidget(obsChannel0, "rf_bandwidth",
											   "RF Bandwidth(MHz)", false);
		rfBandwidthWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(rfBandwidthWidget);

		// Sampling Rate (read-only, shared for OBS section)
		IIOWidget *samplingRateWidget = Adrv9009WidgetFactory::createReadOnlyWidget(
			obsChannel0, "sampling_frequency", "Sampling Rate(MSPS)", false);
		samplingRateWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(samplingRateWidget);

		// LO Source Select (shared dropdown for OBS section)
		IIOWidget *loSourceSelect = Adrv9009WidgetFactory::createComboWidget(
			obsChannel0, "rf_port_select", "rf_port_select_available", "LO Source Select");
		connect(this, &Adrv9009::readRequested, loSourceSelect, &IIOWidget::readAsync);
		sectionControlsLayout->addWidget(loSourceSelect);
	}

	if(auxLo) {
		// AUX PLL LO Frequency (shared control for OBS section)
		IIOWidget *auxLoFreq = Adrv9009WidgetFactory::createRangeWidget(auxLo, "frequency", "[0 1 6000]",
										"AUX PLL LO Frequency(MHz)");
		connect(this, &Adrv9009::readRequested, auxLoFreq, &IIOWidget::readAsync);
		auxLoFreq->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		auxLoFreq->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
		sectionControlsLayout->addWidget(auxLoFreq);
	}
	mainLayout->addLayout(sectionControlsLayout);

	// Side-by-side OBS1/OBS2 layout
	QHBoxLayout *obsLayout = new QHBoxLayout();

	// Create OBS1 and OBS2 channel widgets (voltage2 and voltage3)
	QWidget *obs1Widget = createObsChannelWidget(dev, "OBS1", 2, widget);
	QWidget *obs2Widget = createObsChannelWidget(dev, "OBS2", 3, widget);

	obsLayout->addWidget(obs1Widget);
	obsLayout->addWidget(obs2Widget);

	mainLayout->addLayout(obsLayout);

	qDebug(CAT_ADRV9009) << "OBS RX Chain widgets created successfully";
	return widget;
}

QWidget *Adrv9009::generateFpgaSettingsWidget(QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *fpgaSection = new MenuSectionCollapseWidget(
		title, MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	if(!m_multiDeviceMode) {
		// Single device mode - use first (and only) device
		iio_device *device = m_adrv9009DeviceMap.first();
		QWidget *content = createFpgaSettingsContentForDevice(device, fpgaSection);
		fpgaSection->contentLayout()->addWidget(content);
	} else {
		// Multi-device mode - create tabs
		QTabWidget *deviceTabs = new QTabWidget(fpgaSection);

		QStringList deviceNames = m_adrv9009DeviceMap.keys();
		for(int i = 0; i < deviceNames.size(); i++) {
			QString deviceName = deviceNames[i];
			iio_device *device = m_adrv9009DeviceMap[deviceName];

			QWidget *deviceContent = createFpgaSettingsContentForDevice(device, deviceTabs);
			deviceTabs->addTab(deviceContent, deviceName);
		}

		fpgaSection->contentLayout()->addWidget(deviceTabs);
	}

	return fpgaSection;
}

QWidget *Adrv9009::createFpgaSettingsContentForDevice(iio_device *dev, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	// Find FPGA device corresponding to this ADRV9009 device
	iio_device *fpga_dev = iio_context_find_device(m_ctx, "axi-adrv9009-rx-hpc");
	if(fpga_dev == nullptr) {
		qWarning(CAT_ADRV9009) << "FPGA device (axi-adrv9009-rx-hpc) not found in context";
		return widget;
	}

	QLabel *receiveLabel = new QLabel("Receive", widget);
	Style::setStyle(receiveLabel, style::properties::label::menuMedium);
	mainLayout->addWidget(receiveLabel);

	// Side-by-side RX1/RX2 layout for phase rotation
	QHBoxLayout *fpgaRxLayout = new QHBoxLayout();

	// Create RX1 and RX2 phase rotation widgets
	QWidget *fpgaRx1Widget = createFpgaRxChannelWidget(dev, "RX1", 0, widget);
	QWidget *fpgaRx2Widget = createFpgaRxChannelWidget(dev, "RX2", 1, widget);

	fpgaRxLayout->addWidget(fpgaRx1Widget);
	fpgaRxLayout->addWidget(fpgaRx2Widget);

	mainLayout->addLayout(fpgaRxLayout);

	qDebug(CAT_ADRV9009) << "FPGA Settings widgets created successfully";
	return widget;
}

QWidget *Adrv9009::createRxChannelWidget(iio_device *dev, QString title, int channelIndex, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	// Channel title
	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	mainLayout->addWidget(titleLabel);

	// Find the RX channel (voltage0 for RX1, voltage1 for RX2)
	QString channelId = QString("voltage%1").arg(channelIndex);
	iio_channel *rxChannel = iio_device_find_channel(dev, channelId.toLocal8Bit().data(), false);

	if(!rxChannel) {
		qWarning(CAT_ADRV9009) << "RX channel" << channelId << "not found";
		return widget;
	}

	// Hardware Gain(dB)
	IIOWidget *gainWidget =
		Adrv9009WidgetFactory::createRangeWidget(rxChannel, "hardwaregain", "[0 0.5 30]", "Hardware Gain(dB)");
	connect(this, &Adrv9009::readRequested, gainWidget, &IIOWidget::readAsync);
	mainLayout->addWidget(gainWidget);

	// RSSI (dB) - read-only
	IIOWidget *rssi = Adrv9009WidgetFactory::createReadOnlyWidget(rxChannel, "rssi", "RSSI (dB):");
	connect(this, &Adrv9009::readRequested, rssi, &IIOWidget::readAsync);
	mainLayout->addWidget(rssi);

	// Gain Control - read-only
	IIOWidget *gainControl =
		Adrv9009WidgetFactory::createReadOnlyWidget(rxChannel, "gain_control_mode", "Gain Control:");
	connect(this, &Adrv9009::readRequested, gainControl, &IIOWidget::readAsync);
	mainLayout->addWidget(gainControl);

	// Pin Control checkbox
	IIOWidget *pinMode =
		Adrv9009WidgetFactory::createCheckboxWidget(rxChannel, "gain_control_pin_mode_en", "Pin Control:");
	connect(this, &Adrv9009::readRequested, pinMode, &IIOWidget::readAsync);
	mainLayout->addWidget(pinMode);

	// Powerdown checkbox
	IIOWidget *powerDown = Adrv9009WidgetFactory::createCheckboxWidget(rxChannel, "powerdown", "Powerdown");
	connect(this, &Adrv9009::readRequested, powerDown, &IIOWidget::readAsync);
	mainLayout->addWidget(powerDown);

	mainLayout->addWidget(new QLabel("Tracking:"));

	// Quadrature checkbox
	IIOWidget *quadTracking =
		Adrv9009WidgetFactory::createCheckboxWidget(rxChannel, "quadrature_tracking_en", "Quadrature");
	connect(this, &Adrv9009::readRequested, quadTracking, &IIOWidget::readAsync);
	mainLayout->addWidget(quadTracking);

	// HD2 checkbox
	IIOWidget *hd2Tracking = Adrv9009WidgetFactory::createCheckboxWidget(rxChannel, "hd2_tracking_en", "HD2");
	connect(this, &Adrv9009::readRequested, hd2Tracking, &IIOWidget::readAsync);
	mainLayout->addWidget(hd2Tracking);

	qDebug(CAT_ADRV9009) << title << "channel widget created successfully";
	return widget;
}

QWidget *Adrv9009::createTxChannelWidget(iio_device *dev, QString title, int channelIndex, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	// Channel title
	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	mainLayout->addWidget(titleLabel);

	// Find the TX channel (voltage0 for TX1, voltage1 for TX2)
	QString channelId = QString("voltage%1").arg(channelIndex);
	iio_channel *txChannel = iio_device_find_channel(dev, channelId.toLocal8Bit().data(), true);

	if(!txChannel) {
		qWarning(CAT_ADRV9009) << "TX channel" << channelId << "not found";
		return widget;
	}

	// Attenuation(dB)
	IIOWidget *gainWidget = Adrv9009WidgetFactory::createRangeWidget(txChannel, "hardwaregain", "[0 0.05 41.95]",
									 "Attenuation(dB)");
	connect(this, &Adrv9009::readRequested, gainWidget, &IIOWidget::readAsync);
	gainWidget->setDataToUIConversion([](QString data) { return QString::number(-data.toDouble(), 'f', 2); });
	gainWidget->setUItoDataConversion([](QString data) { return QString::number(-data.toDouble(), 'f', 2); });
	mainLayout->addWidget(gainWidget);

	// Pin Control checkbox
	IIOWidget *pinMode =
		Adrv9009WidgetFactory::createCheckboxWidget(txChannel, "atten_control_pin_mode_en", "Pin Control");
	connect(this, &Adrv9009::readRequested, pinMode, &IIOWidget::readAsync);
	mainLayout->addWidget(pinMode);

	// Powerdown checkbox
	IIOWidget *powerDown = Adrv9009WidgetFactory::createCheckboxWidget(txChannel, "powerdown", "Powerdown");
	connect(this, &Adrv9009::readRequested, powerDown, &IIOWidget::readAsync);
	mainLayout->addWidget(powerDown);

	mainLayout->addWidget(new QLabel("Tracking:"));
	// Quadrature checkbox
	IIOWidget *quadTracking =
		Adrv9009WidgetFactory::createCheckboxWidget(txChannel, "quadrature_tracking_en", "Quadrature");
	connect(this, &Adrv9009::readRequested, quadTracking, &IIOWidget::readAsync);
	mainLayout->addWidget(quadTracking);

	// LO Leakage checkbox
	IIOWidget *loLeakageTracking =
		Adrv9009WidgetFactory::createCheckboxWidget(txChannel, "lo_leakage_tracking_en", "LO Leakage");
	connect(this, &Adrv9009::readRequested, loLeakageTracking, &IIOWidget::readAsync);
	mainLayout->addWidget(loLeakageTracking);

	qDebug(CAT_ADRV9009) << title << "channel widget created successfully";
	return widget;
}

QWidget *Adrv9009::createObsChannelWidget(iio_device *dev, QString title, int channelIndex, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	// Channel title
	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	mainLayout->addWidget(titleLabel);

	// Find the OBS channel (voltage2 for OBS1, voltage3 for OBS2)
	QString channelId = QString("voltage%1").arg(channelIndex);
	iio_channel *obsChannel = iio_device_find_channel(dev, channelId.toLocal8Bit().data(), false);

	if(!obsChannel) {
		qWarning(CAT_ADRV9009) << "OBS channel" << channelId << "not found";
		QLabel *errorLabel = new QLabel("Channel not found", widget);
		mainLayout->addWidget(errorLabel);
		return widget;
	}

	// Create form layout for controls (like iio-osc)
	QFormLayout *formLayout = new QFormLayout();

	// Hardware Gain(dB)
	IIOWidget *gainWidget =
		Adrv9009WidgetFactory::createRangeWidget(obsChannel, "hardwaregain", "[0 1 30]", "Hardware Gain(dB)");
	connect(this, &Adrv9009::readRequested, gainWidget, &IIOWidget::readAsync);
	formLayout->addRow("Hardware Gain(dB):", gainWidget);

	// Tracking: Quadrature checkbox only
	IIOWidget *quadTracking =
		Adrv9009WidgetFactory::createCheckboxWidget(obsChannel, "quadrature_tracking_en", "Quadrature");
	connect(this, &Adrv9009::readRequested, quadTracking, &IIOWidget::readAsync);
	formLayout->addRow("Tracking:", quadTracking);

	// Powerdown checkbox
	IIOWidget *powerDown = Adrv9009WidgetFactory::createCheckboxWidget(obsChannel, "powerdown", "Powerdown");
	connect(this, &Adrv9009::readRequested, powerDown, &IIOWidget::readAsync);
	formLayout->addRow("Powerdown:", powerDown);

	mainLayout->addLayout(formLayout);

	qDebug(CAT_ADRV9009) << title << "channel widget created successfully";
	return widget;
}

QWidget *Adrv9009::createFpgaRxChannelWidget(iio_device *dev, QString title, int channelIndex, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	// Channel title
	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	mainLayout->addWidget(titleLabel);

	// Phase Rotation MenuSpinbox (proper Scopy styling)
	gui::MenuSpinbox *phaseSpinBox =
		new gui::MenuSpinbox("Phase Rotation", 0, "°", -180, 180, true, false, false, widget);
	phaseSpinBox->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	phaseSpinBox->setScalingEnabled(false);
	phaseSpinBox->enableRangeLimits(false);

	// Connect to phase rotation functions
	connect(phaseSpinBox, &gui::MenuSpinbox::valueChanged, this,
		[=](double degrees) { writePhase(dev, channelIndex, (int)degrees); });

	// Add to refresh handling
	connect(this, &Adrv9009::readRequested, this, [=]() { readPhase(dev, channelIndex, phaseSpinBox); });

	mainLayout->addWidget(phaseSpinBox);

	readPhase(dev, channelIndex, phaseSpinBox);

	qDebug(CAT_ADRV9009) << "FPGA" << title << "channel widget created successfully";
	return widget;
}

void Adrv9009::writePhase(iio_device *fpgaDev, int channelIndex, int degrees)
{
	// Convert degrees to radians
	double phase = degrees * 2.0 * M_PI / 360.0;

	// Find I and Q channels for this RX
	QString i_ch = QString("voltage%1_i").arg(channelIndex);
	QString q_ch = QString("voltage%1_q").arg(channelIndex);

	iio_channel *i_chn = iio_device_find_channel(fpgaDev, i_ch.toLatin1(), false);
	iio_channel *q_chn = iio_device_find_channel(fpgaDev, q_ch.toLatin1(), false);

	if(i_chn && q_chn) {
		// Write the 4 calculated values (exact iio-osc formula)
		iio_channel_attr_write_double(i_chn, "calibscale", cos(phase));
		iio_channel_attr_write_double(i_chn, "calibphase", -sin(phase));
		iio_channel_attr_write_double(q_chn, "calibscale", cos(phase));
		iio_channel_attr_write_double(q_chn, "calibphase", sin(phase));

		qDebug(CAT_ADRV9009) << "Phase rotation set to" << degrees << "degrees for channel" << channelIndex;
	} else {
		qWarning(CAT_ADRV9009) << "Failed to find FPGA channels for phase rotation";
	}
}

void Adrv9009::readPhase(iio_device *fpgaDev, int channelIndex, gui::MenuSpinbox *spinBox)
{
	// Exact copy of iio-osc rx_phase_rotation_update algorithm
	QString i_ch = QString("voltage%1_i").arg(channelIndex);
	QString q_ch = QString("voltage%1_q").arg(channelIndex);

	iio_channel *i_chn = iio_device_find_channel(fpgaDev, i_ch.toLatin1(), false);
	iio_channel *q_chn = iio_device_find_channel(fpgaDev, q_ch.toLatin1(), false);

	if(i_chn && q_chn && spinBox) {
		double val[4];
		if(iio_channel_attr_read_double(i_chn, "calibscale", &val[0]) == 0 &&
		   iio_channel_attr_read_double(i_chn, "calibphase", &val[1]) == 0 &&
		   iio_channel_attr_read_double(q_chn, "calibscale", &val[2]) == 0 &&
		   iio_channel_attr_read_double(q_chn, "calibphase", &val[3]) == 0) {

			// Exact iio-osc reverse calculations
			val[0] = acos(val[0]) * 360.0 / (2.0 * M_PI);
			val[1] = asin(-1.0 * val[1]) * 360.0 / (2.0 * M_PI);
			val[2] = acos(val[2]) * 360.0 / (2.0 * M_PI);
			val[3] = asin(val[3]) * 360.0 / (2.0 * M_PI);

			// Exact iio-osc sign handling
			if(val[1] < 0.0)
				val[0] *= -1.0;

			// Average like iio-osc
			double degrees = (val[0] + val[1] + val[2] + val[3]) / 4.0;
			spinBox->setValue(degrees);

			qDebug(CAT_ADRV9009) << "Read phase rotation:" << (int)round(degrees) << "degrees for channel"
					     << channelIndex << "vals:" << val[0] << val[1] << val[2] << val[3];
		} else {
			qWarning(CAT_ADRV9009)
				<< "Failed to read phase rotation attributes for channel" << channelIndex;
		}
	}
}
