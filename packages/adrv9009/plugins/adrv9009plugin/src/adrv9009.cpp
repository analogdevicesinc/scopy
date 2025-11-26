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
	, m_iio_dev(nullptr)
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

	m_iio_dev = iio_context_find_device(m_ctx, "adrv9009-phy");

	if(m_iio_dev != nullptr) {
		// Global Settings - Collapsible Section
		MenuSectionCollapseWidget *globalSection =
			new MenuSectionCollapseWidget("ADRV9009 Global Settings", MenuCollapseSection::MHCW_ARROW,
						      MenuCollapseSection::MHW_BASEWIDGET, controlsScrollWidget);
		globalSection->contentLayout()->addWidget(
			generateGlobalSettingsWidget(m_iio_dev, "ADRV9009 Global Settings", globalSection));
		controlsScrollLayout->addWidget(globalSection);

		// Receive Chain - Collapsible Section
		MenuSectionCollapseWidget *rxSection =
			new MenuSectionCollapseWidget("ADRV9009 Receive Chain", MenuCollapseSection::MHCW_ARROW,
						      MenuCollapseSection::MHW_BASEWIDGET, controlsScrollWidget);
		rxSection->contentLayout()->addWidget(
			generateRxChainWidget(m_iio_dev, "ADRV9009 Receive Chain", rxSection));
		controlsScrollLayout->addWidget(rxSection);

		// Transmit Chain - Collapsible Section
		MenuSectionCollapseWidget *txSection =
			new MenuSectionCollapseWidget("ADRV9009 Transmit Chain", MenuCollapseSection::MHCW_ARROW,
						      MenuCollapseSection::MHW_BASEWIDGET, controlsScrollWidget);
		txSection->contentLayout()->addWidget(
			generateTxChainWidget(m_iio_dev, "ADRV9009 Transmit Chain", txSection));
		controlsScrollLayout->addWidget(txSection);

		// Observation RX - Collapsible Section
		MenuSectionCollapseWidget *obsSection =
			new MenuSectionCollapseWidget("ADRV9009 Observation RX", MenuCollapseSection::MHCW_ARROW,
						      MenuCollapseSection::MHW_BASEWIDGET, controlsScrollWidget);
		obsSection->contentLayout()->addWidget(
			generateObsRxChainWidget(m_iio_dev, "ADRV9009 Observation RX", obsSection));
		controlsScrollLayout->addWidget(obsSection);

		// FPGA Settings - Collapsible Section
		iio_device *fpga_dev = iio_context_find_device(m_ctx, "axi-adrv9009-rx-hpc");
		if(fpga_dev != nullptr) {
			MenuSectionCollapseWidget *fpgaSection = new MenuSectionCollapseWidget(
				"FPGA Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET,
				controlsScrollWidget);
			fpgaSection->contentLayout()->addWidget(
				generateFpgaSettingsWidget(fpga_dev, "FPGA Settings", fpgaSection));
			controlsScrollLayout->addWidget(fpgaSection);
		} else {
			qWarning(CAT_ADRV9009) << "FPGA device (axi-adrv9009-rx-hpc) not found in context";
		}

		controlsScrollLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	} else {
		qWarning(CAT_ADRV9009) << "ADRV9009 device not found in context";
		return;
	}

	m_tool->addWidgetToCentralContainerHelper(m_centralWidget);
}

QWidget *Adrv9009::generateGlobalSettingsWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	// Title is now handled by the collapsible section header
	// No need for individual section titles

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
		IIOWidget *trxLoFreq = createRangeWidget(trxLo, "frequency", "[70 1 6000]", "Frequency(MHz)");
		trxLoFreq->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		trxLoFreq->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });

		trxLoLayout->addWidget(trxLoFreq);

		// Frequency Hopping Mode
		trxLoLayout->addWidget(
			createCheckboxWidget(trxLo, "frequency_hopping_mode_enable", "Frequency Hopping Mode"));
	}

	layout->addWidget(trxLoWidget, 2, 0);

	layout->addWidget(generateCalibrationWidget(widget), 2, 1);

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

		// Write profile to device
		int ret = iio_device_attr_write_raw(m_iio_dev, "profile_config", buffer.constData(), buffer.size());

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

QWidget *Adrv9009::generateCalibrationWidget(QWidget *parent)
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
	IIOWidget *calRxQec = createCheckboxWidget(nullptr, "calibrate_rx_qec_en", "CAL RX QEC");
	IIOWidget *calTxQec = createCheckboxWidget(nullptr, "calibrate_tx_qec_en", "CAL TX QEC");
	IIOWidget *calTxLol = createCheckboxWidget(nullptr, "calibrate_tx_lol_en", "CAL TX LOL");
	IIOWidget *calTxLolExt = createCheckboxWidget(nullptr, "calibrate_tx_lol_ext_en", "CAL TX LOL Ext.");

	calGridLayout->addWidget(calRxQec, 0, 0);
	calGridLayout->addWidget(calTxQec, 0, 1);
	calGridLayout->addWidget(calTxLol, 0, 2);
	calGridLayout->addWidget(calTxLolExt, 0, 3);

	// Row 2 checkboxes
	IIOWidget *calRxPhaseCorr =
		createCheckboxWidget(nullptr, "calibrate_rx_phase_correction_en", "CAL RX PHASE CORR");
	IIOWidget *calFhm = createCheckboxWidget(nullptr, "calibrate_fhm_en", "CAL FHM");

	calGridLayout->addWidget(calRxPhaseCorr, 1, 0);
	calGridLayout->addWidget(calFhm, 1, 1);

	// CALIBRATE button
	QPushButton *calibrateButton = new QPushButton("CALIBRATE", calibrationsWidget);
	Style::setStyle(calibrateButton, style::properties::button::basicButton);
	calGridLayout->addWidget(calibrateButton, 1, 3);

	connect(calibrateButton, &QPushButton::clicked, this, [=] {
		// Trigger calibration
		int ret = iio_device_attr_write_bool(m_iio_dev, "calibrate", true);
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

QWidget *Adrv9009::generateRxChainWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	// Title is now handled by the collapsible section header
	// No need for individual section titles

	if(dev == nullptr) {
		qWarning(CAT_ADRV9009) << "No ADRV9009 device found for RX chain";
		return widget;
	}

	// Section-level controls
	QHBoxLayout *sectionControlsLayout = new QHBoxLayout();

	iio_channel *rxChannel0 = iio_device_find_channel(dev, "voltage0", false);

	if(rxChannel0) {
		// RF Bandwidth (read-only, shared for RX section)
		IIOWidget *rfBandwidthWidget =
			createReadOnlyWidget(rxChannel0, "rf_bandwidth", "RF Bandwidth(MHz)", false);
		rfBandwidthWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(rfBandwidthWidget);

		// Sampling Rate (read-only, shared for RX section)
		IIOWidget *samplingRateWidget =
			createReadOnlyWidget(rxChannel0, "sampling_frequency", "Sampling Rate(MSPS)", false);
		samplingRateWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(samplingRateWidget);

		// Gain Control Modes (shared dropdown for RX section)
		IIOWidget *gainControlModes = createComboWidget(rxChannel0, "gain_control_mode",
								"gain_control_mode_available", "Gain Control Modes");
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

QWidget *Adrv9009::generateTxChainWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	// Title is now handled by the collapsible section header
	// No need for individual section titles

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
		IIOWidget *rfBandwidthWidget =
			createReadOnlyWidget(txChannel0, "rf_bandwidth", "RF Bandwidth(MHz)", false);
		rfBandwidthWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(rfBandwidthWidget);

		// Sampling Rate (read-only, shared for TX section)
		IIOWidget *samplingRateWidget =
			createReadOnlyWidget(txChannel0, "sampling_frequency", "Sampling Rate(MSPS)", false);
		samplingRateWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(samplingRateWidget);

		// PA Protection (shared checkbox for TX section)
		IIOWidget *paProtection = createCheckboxWidget(txChannel0, "pa_protection_en", "PA Protection");
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

QWidget *Adrv9009::generateObsRxChainWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	// Title is now handled by the collapsible section header
	// No need for individual section titles

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
		IIOWidget *rfBandwidthWidget =
			createReadOnlyWidget(obsChannel0, "rf_bandwidth", "RF Bandwidth(MHz)", false);
		rfBandwidthWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(rfBandwidthWidget);

		// Sampling Rate (read-only, shared for OBS section)
		IIOWidget *samplingRateWidget =
			createReadOnlyWidget(obsChannel0, "sampling_frequency", "Sampling Rate(MSPS)", false);
		samplingRateWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControlsLayout->addWidget(samplingRateWidget);

		// LO Source Select (shared dropdown for OBS section)
		IIOWidget *loSourceSelect = createComboWidget(obsChannel0, "rf_port_select", "rf_port_select_available",
							      "LO Source Select");
		sectionControlsLayout->addWidget(loSourceSelect);
	}

	if(auxLo) {
		// AUX PLL LO Frequency (shared control for OBS section)
		IIOWidget *auxLoFreq = createRangeWidget(auxLo, "frequency", "[0 1 6000]", "AUX PLL LO Frequency(MHz)");
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

QWidget *Adrv9009::generateFpgaSettingsWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	// Title is now handled by the collapsible section header
	// No need for individual section titles

	if(dev == nullptr) {
		qWarning(CAT_ADRV9009) << "No FPGA device found for FPGA settings";
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
	IIOWidget *gainWidget = createRangeWidget(rxChannel, "hardwaregain", "[0 0.5 30]", "Hardware Gain(dB)");
	mainLayout->addWidget(gainWidget);

	// RSSI (dB) - read-only
	IIOWidget *rssi = createReadOnlyWidget(rxChannel, "rssi", "RSSI (dB):");
	mainLayout->addWidget(rssi);

	// Gain Control - read-only
	IIOWidget *gainControl = createReadOnlyWidget(rxChannel, "gain_control_mode", "Gain Control:");
	mainLayout->addWidget(gainControl);

	// Pin Control checkbox
	IIOWidget *pinMode = createCheckboxWidget(rxChannel, "gain_control_pin_mode_en", "Pin Control:");
	mainLayout->addWidget(pinMode);

	// Powerdown checkbox
	IIOWidget *powerDown = createCheckboxWidget(rxChannel, "powerdown", "Powerdown");
	mainLayout->addWidget(powerDown);

	mainLayout->addWidget(new QLabel("Tracking:"));

	// Quadrature checkbox
	IIOWidget *quadTracking = createCheckboxWidget(rxChannel, "quadrature_tracking_en", "Quadrature");
	mainLayout->addWidget(quadTracking);

	// HD2 checkbox
	IIOWidget *hd2Tracking = createCheckboxWidget(rxChannel, "hd2_tracking_en", "HD2");
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
	IIOWidget *gainWidget = createRangeWidget(txChannel, "hardwaregain", "[0 0.05 41.95]", "Attenuation(dB)");
	gainWidget->setDataToUIConversion([](QString data) { return QString::number(-data.toDouble(), 'f', 2); });
	gainWidget->setUItoDataConversion([](QString data) { return QString::number(-data.toDouble(), 'f', 2); });
	mainLayout->addWidget(gainWidget);

	// Pin Control checkbox
	IIOWidget *pinMode = createCheckboxWidget(txChannel, "atten_control_pin_mode_en", "Pin Control");
	mainLayout->addWidget(pinMode);

	// Powerdown checkbox
	IIOWidget *powerDown = createCheckboxWidget(txChannel, "powerdown", "Powerdown");
	mainLayout->addWidget(powerDown);

	mainLayout->addWidget(new QLabel("Tracking:"));
	// Quadrature checkbox
	IIOWidget *quadTracking = createCheckboxWidget(txChannel, "quadrature_tracking_en", "Quadrature");
	mainLayout->addWidget(quadTracking);

	// LO Leakage checkbox
	IIOWidget *loLeakageTracking = createCheckboxWidget(txChannel, "lo_leakage_tracking_en", "LO Leakage");
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
	IIOWidget *gainWidget = createRangeWidget(obsChannel, "hardwaregain", "[0.0 1 30.0]", "Hardware Gain(dB)");
	formLayout->addRow("Hardware Gain(dB):", gainWidget);

	// Tracking: Quadrature checkbox only
	IIOWidget *quadTracking = createCheckboxWidget(obsChannel, "quadrature_tracking_en", "Quadrature");
	formLayout->addRow("Tracking:", quadTracking);

	// Powerdown checkbox
	IIOWidget *powerDown = createCheckboxWidget(obsChannel, "powerdown", "Powerdown");
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

scopy::IIOWidget *Adrv9009::createComboWidget(iio_channel *ch, const QString &attr, const QString &availableAttr,
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
		connect(this, &Adrv9009::readRequested, widget, &IIOWidget::readAsync);
	}
	return widget;
}

scopy::IIOWidget *Adrv9009::createRangeWidget(iio_channel *ch, const QString &attr, const QString &range,
					      const QString &title)
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
		connect(this, &Adrv9009::readRequested, widget, &IIOWidget::readAsync);
	}
	return widget;
}

scopy::IIOWidget *Adrv9009::createCheckboxWidget(iio_channel *ch, const QString &attr, const QString &label)
{
	IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
				    .device(m_iio_dev)
				    .channel(ch)
				    .attribute(attr)
				    .title(label)
				    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				    .buildSingle();

	if(widget) {
		connect(this, &Adrv9009::readRequested, widget, &IIOWidget::readAsync);
		widget->showProgressBar(false);
	}
	return widget;
}

scopy::IIOWidget *Adrv9009::createReadOnlyWidget(iio_channel *ch, const QString &attr, const QString &title,
						 bool compactMode)
{
	IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
				    .device(m_iio_dev)
				    .channel(ch)
				    .attribute(attr)
				    .title(title)
				    .compactMode(compactMode)
				    .buildSingle();

	if(widget) {
		widget->setEnabled(false);
		widget->showProgressBar(false);

		connect(this, &Adrv9009::readRequested, widget, &IIOWidget::readAsync);
	}
	return widget;
}
