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

#include "ad9371.h"
#include <QLabel>
#include <QScrollArea>
#include <QSpacerItem>
#include <QGridLayout>
#include <QTimer>
#include <QCheckBox>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QPixmap>
#include <cmath>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/filebrowserwidget.h>
#include <pkg-manager/pkgmanager.h>
#include <QFile>
#include <style.h>
#include <QFutureWatcher>
#include <QLoggingCategory>
#include <qtconcurrentrun.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <guistrategy/comboguistrategy.h>

Q_LOGGING_CATEGORY(CAT_AD9371, "AD9371");

using namespace scopy;
using namespace scopy::ad9371;

Ad9371::Ad9371(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_widgetGroup(group)
	, m_tool(nullptr)
	, m_refreshButton(nullptr)
	, m_centralWidget(nullptr)
{
	setupUi();
	connect(this, &Ad9371::readRequested, this, &Ad9371::readCalibrationFromHardware);
	readCalibrationFromHardware();
	qDebug(CAT_AD9371) << "AD9371 tool initialized successfully";
}

Ad9371::~Ad9371() {}

void Ad9371::detectDevices()
{
	if(!m_ctx)
		return;

	m_dev = iio_context_find_device(m_ctx, "ad9371-phy");
	m_dds = iio_context_find_device(m_ctx, "axi-ad9371-tx-hpc");
	m_cap = iio_context_find_device(m_ctx, "axi-ad9371-rx-hpc");
	m_udcRx = iio_context_find_device(m_ctx, "adf4351-udc-rx-pmod");
	m_udcTx = iio_context_find_device(m_ctx, "adf4351-udc-tx-pmod");
	m_hasUdc = (m_udcRx != nullptr && m_udcTx != nullptr);

	if(m_dev) {
		iio_channel *ch1 = iio_device_find_channel(m_dev, "voltage1", false);
		m_is2Rx2Tx = ch1 && iio_channel_find_attr(ch1, "hardwaregain");

		iio_channel *txCh0 = iio_device_find_channel(m_dev, "voltage0", true);
	}

	qDebug(CAT_AD9371) << "Devices - PHY:" << (m_dev != nullptr) << "DDS:" << (m_dds != nullptr)
			   << "CAP:" << (m_cap != nullptr) << "UDC:" << m_hasUdc << "2Rx2Tx:" << m_is2Rx2Tx;
}

const char *Ad9371::resolveFreqAttrName(iio_channel *ch, const char *fallback)
{
	if(iio_channel_find_attr(ch, "frequency"))
		return "frequency";
	return fallback;
}

void Ad9371::setupUi()
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

	// Phase rotation reads directly on refresh (computed from 4 IIO attrs, not an IIOWidget)
	connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
		if(m_phaseSpinbox1)
			readPhase(m_cap, 0, m_phaseSpinbox1);
		if(m_phaseSpinbox2)
			readPhase(m_cap, 1, m_phaseSpinbox2);
	});

	QStackedWidget *stackedWidget = new QStackedWidget(this);

	// Controls view
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
		qWarning(CAT_AD9371) << "No context provided";
		return;
	}

	detectDevices();

	if(!m_dev) {
		qWarning(CAT_AD9371) << "AD9371 PHY device not found";
		return;
	}

	controlsScrollLayout->addWidget(generateGlobalSettingsWidget("AD9371 Global Settings", controlsScrollWidget));
	controlsScrollLayout->addWidget(generateRxChainWidget("AD9371 Receive Chain", controlsScrollWidget));
	controlsScrollLayout->addWidget(generateTxChainWidget("AD9371 Transmit Chain", controlsScrollWidget));
	controlsScrollLayout->addWidget(
		generateObsRxChainWidget("AD9371 Observation/Sniffer Receive Chain", controlsScrollWidget));
	controlsScrollLayout->addWidget(generateFpgaSettingsWidget("FPGA Settings", controlsScrollWidget));

	// Connect FPGA frequency combos to sampling rate labels
	if(m_fpgaTxFreqCombo && m_txSampRateLabel) {
		connect(m_fpgaTxFreqCombo, &IIOWidget::currentStateChanged, this, [this](IIOWidget::State s) {
			if(s == IIOWidget::Correct)
				m_txSampRateLabel->readAsync();
		});
	}
	if(m_fpgaRxFreqCombo && m_rxSampRateLabel) {
		connect(m_fpgaRxFreqCombo, &IIOWidget::currentStateChanged, this, [this](IIOWidget::State s) {
			if(s == IIOWidget::Correct)
				m_rxSampRateLabel->readAsync();
		});
	}

	controlsScrollLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	// Block diagram view
	m_blockDiagramWidget = new QWidget(this);
	m_blockDiagramWidget->setStyleSheet("background-color: white;");
	QVBoxLayout *blockDiagramLayout = new QVBoxLayout(m_blockDiagramWidget);
	m_blockDiagramWidget->setLayout(blockDiagramLayout);

	QWidget *blockDiagramContent = new QWidget(this);
	QVBoxLayout *blockDiagramContentLayout = new QVBoxLayout(blockDiagramContent);
	blockDiagramContent->setLayout(blockDiagramContentLayout);

	QScrollArea *blockDiagramScrollArea = new QScrollArea(this);
	blockDiagramScrollArea->setWidgetResizable(true);
	blockDiagramScrollArea->setWidget(blockDiagramContent);
	blockDiagramLayout->addWidget(blockDiagramScrollArea);

	QLabel *blockDiagram = new QLabel(m_blockDiagramWidget);
	blockDiagramContentLayout->addWidget(blockDiagram);
	blockDiagram->setAlignment(Qt::AlignCenter);
	QPixmap pixmap(":/ad9371/AD9371.png");
	blockDiagram->setPixmap(pixmap);

	// Stacked widget: Controls + Block Diagram
	stackedWidget->addWidget(m_centralWidget);
	stackedWidget->addWidget(m_blockDiagramWidget);
	m_tool->addWidgetToCentralContainerHelper(stackedWidget);

	// Navigation buttons
	QButtonGroup *viewButtons = new QButtonGroup(this);
	viewButtons->setExclusive(true);

	QPushButton *controlsBtn = new QPushButton("Controls", this);
	controlsBtn->setCheckable(true);
	controlsBtn->setChecked(true);
	Style::setStyle(controlsBtn, style::properties::button::blueGrayButton);
	connect(controlsBtn, &QPushButton::clicked, this,
		[stackedWidget, this]() { stackedWidget->setCurrentWidget(m_centralWidget); });

	QPushButton *blockDiagramBtn = new QPushButton("Block Diagram", this);
	blockDiagramBtn->setCheckable(true);
	Style::setStyle(blockDiagramBtn, style::properties::button::blueGrayButton);
	connect(blockDiagramBtn, &QPushButton::clicked, this,
		[stackedWidget, this]() { stackedWidget->setCurrentWidget(m_blockDiagramWidget); });

	viewButtons->addButton(controlsBtn);
	viewButtons->addButton(blockDiagramBtn);

	m_tool->addWidgetToTopContainerHelper(controlsBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(blockDiagramBtn, TTA_LEFT);

	// 1-second auto-refresh for RSSI and hardware gain (issues #1 and #2)
	m_liveTimer = new QTimer(this);
	m_liveTimer->setInterval(1000);
	connect(m_liveTimer, &QTimer::timeout, this, [this]() {
		for(IIOWidget *w : m_liveWidgets) {
			w->readAsync();
		}
	});
	m_liveTimer->start();
}

// --- Global Settings---
QWidget *Ad9371::generateGlobalSettingsWidget(QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *content = new QWidget(section);
	Style::setBackgroundColor(content, json::theme::background_primary);
	Style::setStyle(content, style::properties::widget::border_interactive);
	QVBoxLayout *mainLayout = new QVBoxLayout(content);
	content->setLayout(mainLayout);

	QGridLayout *topGrid = new QGridLayout();

	// #1: ENSM Mode combo (device attribute)
	IIOWidget *ensmWidget =
		Ad9371WidgetFactory::createComboWidget(m_dev, "ensm_mode", "ensm_mode_available", "ENSM Mode", content);
	if(m_widgetGroup)
		m_widgetGroup->add(ensmWidget);
	connect(this, &Ad9371::readRequested, ensmWidget, &IIOWidget::readAsync);
	topGrid->addWidget(ensmWidget, 1, 0);

	// Load Profile section
	QLabel *loadProfileLabel = new QLabel("Load Profile", content);
	Style::setStyle(loadProfileLabel, style::properties::label::menuMedium);
	topGrid->addWidget(loadProfileLabel, 0, 1);

	QFileInfoList filterFiles = PkgManager::listFilesInfo(QStringList() << "ad9371-filters");
	QString defaultDir = (filterFiles.size() > 0) ? filterFiles.first().absolutePath() : PkgManager::packagesPath();

	FileBrowserWidget *profileWidget = new FileBrowserWidget(FileBrowserWidget::OPEN_FILE, content);
	profileWidget->setFilter("Profile files (*.txt);;All files (*.*)");
	profileWidget->setBaseDirectory(defaultDir);
	topGrid->addWidget(profileWidget, 1, 1);
	profileWidget->lineEdit()->setReadOnly(true);

	connect(profileWidget->lineEdit(), &QLineEdit::textChanged, this, &Ad9371::loadProfileFromFile);

	// Up/Down Converter checkbox (only if ADF4351 devices present)
	if(m_hasUdc) {
		MenuOnOffSwitch *udcCheckbox = new MenuOnOffSwitch("Up/Down Converter", content);
		connect(udcCheckbox->onOffswitch(), &QCheckBox::toggled, this, &Ad9371::onUdcToggled);
		topGrid->addWidget(udcCheckbox, 1, 2);
	}

	mainLayout->addLayout(topGrid);

	// Calibrations sub-widget (styled card)
	QWidget *calibrationsWidget = new QWidget(content);
	Style::setBackgroundColor(calibrationsWidget, json::theme::background_primary);
	Style::setStyle(calibrationsWidget, style::properties::widget::border_interactive);
	QVBoxLayout *calibrationsLayout = new QVBoxLayout(calibrationsWidget);

	QLabel *calTitle = new QLabel("Calibrations", calibrationsWidget);
	Style::setStyle(calTitle, style::properties::label::menuMedium);
	calibrationsLayout->addWidget(calTitle);

	QGridLayout *calGrid = new QGridLayout();

	// #5: calibrate_rx_qec_en
	m_calRxQec = new MenuOnOffSwitch("CAL RX QEC", calibrationsWidget);
	calGrid->addWidget(m_calRxQec, 0, 0);

	// #6: calibrate_tx_qec_en
	m_calTxQec = new MenuOnOffSwitch("CAL TX QEC", calibrationsWidget);
	calGrid->addWidget(m_calTxQec, 0, 1);

	// #7: calibrate_tx_lol_en
	m_calTxLol = new MenuOnOffSwitch("CAL TX LOL", calibrationsWidget);
	calGrid->addWidget(m_calTxLol, 0, 2);

	// #8: calibrate_tx_lol_ext_en
	m_calTxLolExt = new MenuOnOffSwitch("CAL TX LOL Ext.", calibrationsWidget);
	calGrid->addWidget(m_calTxLolExt, 0, 3);

	// #9: Calibrate button
	QPushButton *calibrateButton = new QPushButton("CALIBRATE", calibrationsWidget);
	Style::setStyle(calibrateButton, style::properties::button::basicButton);
	connect(calibrateButton, &QPushButton::clicked, this, &Ad9371::writeCalibrationToHardware);
	calGrid->addWidget(calibrateButton, 0, 3);

	calibrationsLayout->addLayout(calGrid);
	mainLayout->addWidget(calibrationsWidget);

	section->contentLayout()->addWidget(content);
	return section;
}

void Ad9371::loadProfileFromFile(QString filePath)
{
	if(filePath.isEmpty()) {
		qWarning(CAT_AD9371) << "Profile loading failed, no file path provided";
		return;
	}
	QFile file(filePath);
	if(!file.open(QIODevice::ReadOnly)) {
		qWarning(CAT_AD9371) << "Failed to open profile:" << file.errorString();
		return;
	}
	QByteArray buffer = file.readAll();
	file.close();

	iio_context_set_timeout(m_ctx, 30000);
	int ret = iio_device_attr_write_raw(m_dev, "profile_config", buffer.constData(), buffer.size());
	iio_context_set_timeout(m_ctx, 3000);
	if(ret < 0)
		qWarning(CAT_AD9371) << "Profile loading failed, error:" << ret;
	else {
		qDebug(CAT_AD9371) << "Profile loaded successfully";
		Q_EMIT readRequested();
	}
}

void Ad9371::readCalibrationFromHardware()
{
	if(!m_dev)
		return;
	bool val;
	auto readSwitch = [&](MenuOnOffSwitch *sw, const char *attr) {
		if(!sw)
			return;
		int ret = iio_device_attr_read_bool(m_dev, attr, &val);
		if(ret >= 0)
			sw->onOffswitch()->setChecked(val);
	};
	readSwitch(m_calRxQec, "calibrate_rx_qec_en");
	readSwitch(m_calTxQec, "calibrate_tx_qec_en");
	readSwitch(m_calTxLol, "calibrate_tx_lol_en");
	readSwitch(m_calTxLolExt, "calibrate_tx_lol_ext_en");
}

void Ad9371::writeCalibrationToHardware()
{
	if(!m_dev)
		return;
	auto writeSwitch = [&](MenuOnOffSwitch *sw, const char *attr) {
		if(!sw)
			return;
		iio_device_attr_write_bool(m_dev, attr, sw->onOffswitch()->isChecked());
	};
	writeSwitch(m_calRxQec, "calibrate_rx_qec_en");
	writeSwitch(m_calTxQec, "calibrate_tx_qec_en");
	writeSwitch(m_calTxLol, "calibrate_tx_lol_en");
	writeSwitch(m_calTxLolExt, "calibrate_tx_lol_ext_en");
	int ret = iio_device_attr_write_bool(m_dev, "calibrate", true);
	if(ret < 0)
		qWarning(CAT_AD9371) << "Calibration failed:" << ret;
	else {
		qDebug(CAT_AD9371) << "Calibration triggered";
		Q_EMIT readRequested();
	}
}

// --- Receive Chain ---
QWidget *Ad9371::generateRxChainWidget(QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *content = new QWidget(section);
	Style::setBackgroundColor(content, json::theme::background_primary);
	Style::setStyle(content, style::properties::widget::border_interactive);
	QVBoxLayout *mainLayout = new QVBoxLayout(content);
	content->setLayout(mainLayout);

	iio_channel *rxCh0 = iio_device_find_channel(m_dev, "voltage0", false);
	iio_channel *rxCh1 = m_is2Rx2Tx ? iio_device_find_channel(m_dev, "voltage1", false) : nullptr;

	// Section-level controls
	QHBoxLayout *sectionControls = new QHBoxLayout();
	IIOWidget *gainMode = nullptr;

	if(rxCh0) {
		// #21: RF Bandwidth (read-only, MHz)
		IIOWidget *rfBw =
			Ad9371WidgetFactory::createReadOnlyWidget(rxCh0, "rf_bandwidth", "RF Bandwidth(MHz)", false);
		if(m_widgetGroup)
			m_widgetGroup->add(rfBw);
		connect(this, &Ad9371::readRequested, rfBw, &IIOWidget::readAsync);
		rfBw->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		char rfBwBuf[256] = {0};
		int rfBwRet = iio_channel_attr_read(rxCh0, "rf_bandwidth", rfBwBuf, sizeof(rfBwBuf));
		if(rfBwRet < 0 || strcmp(rfBwBuf, "ERROR") == 0) {
			rfBw->setEnabled(false);
			rfBw->getUiStrategy()->setInfoMessage("Can't access attribute rf_bandwidth");
		}
		sectionControls->addWidget(rfBw);

		// #22: Sampling Frequency (read-only, MSPS)
		IIOWidget *sampRate = Ad9371WidgetFactory::createReadOnlyWidget(rxCh0, "sampling_frequency",
										"Sampling Rate(MSPS)", false);
		if(m_widgetGroup)
			m_widgetGroup->add(sampRate);
		connect(this, &Ad9371::readRequested, sampRate, &IIOWidget::readAsync);
		sampRate->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControls->addWidget(sampRate);
		m_rxSampRateLabel = sampRate;

		// #18: RX LO Frequency (altvoltage0, MHz scale)
		iio_channel *rxLo = iio_device_find_channel(m_dev, "altvoltage0", true);
		if(rxLo) {
			const char *freqAttr = resolveFreqAttrName(rxLo, "RX_LO_frequency");
			IIOWidget *rxLoFreq = Ad9371WidgetFactory::createRangeWidget(rxLo, freqAttr, "[70 1 6000]",
										     "RX LO Frequency(MHz)");
			if(m_widgetGroup)
				m_widgetGroup->add(rxLoFreq);
			connect(this, &Ad9371::readRequested, rxLoFreq, &IIOWidget::readAsync);
			rxLoFreq->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
			rxLoFreq->setUItoDataConversion(
				[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
			sectionControls->addWidget(rxLoFreq);
			m_rxLoWidget = rxLoFreq;
		}

		// #10: Gain Control Mode
		gainMode = Ad9371WidgetFactory::createComboWidget(rxCh0, "gain_control_mode",
								  "gain_control_mode_available", "Gain Control Mode");
		if(m_widgetGroup)
			m_widgetGroup->add(gainMode);
		connect(this, &Ad9371::readRequested, gainMode, &IIOWidget::readAsync);
		sectionControls->addWidget(gainMode);

		// Gain Control SYNC Pulse (debug attribute)
		IIOWidget *gcSyncPulse = Ad9371WidgetFactory::createDebugCheckboxWidget(
			m_dev, "adi,rx-agc-conf-agc-enable-sync-pulse-for-gain-counter", "Gain Control SYNC Pulse");
		if(m_widgetGroup)
			m_widgetGroup->add(gcSyncPulse);
		connect(this, &Ad9371::readRequested, gcSyncPulse, &IIOWidget::readAsync);
		if(iio_device_find_debug_attr(m_dev, "adi,rx-agc-conf-agc-enable-sync-pulse-for-gain-counter") ==
		   nullptr) {
			gcSyncPulse->setEnabled(false);
			gcSyncPulse->getUiStrategy()->setInfoMessage(
				"Can't access attribute adi,rx-agc-conf-agc-enable-sync-pulse-for-gain-counter");
		}
		sectionControls->addWidget(gcSyncPulse);
	}
	mainLayout->addLayout(sectionControls);

	// Per-channel layout (side-by-side RX1/RX2)
	QHBoxLayout *channelLayout = new QHBoxLayout();

	// RX1 column
	if(rxCh0) {
		QWidget *rx1Widget = new QWidget(content);
		Style::setBackgroundColor(rx1Widget, json::theme::background_primary);
		Style::setStyle(rx1Widget, style::properties::widget::border_interactive);
		QVBoxLayout *rx1Layout = new QVBoxLayout(rx1Widget);
		rx1Widget->setLayout(rx1Layout);

		QLabel *rx1Title = new QLabel("RX1", rx1Widget);
		Style::setStyle(rx1Title, style::properties::label::menuBig);
		rx1Layout->addWidget(rx1Title);

		// #13: hardwaregain (voltage0 in) [0, 30, 0.5]
		IIOWidget *hwGain1 = Ad9371WidgetFactory::createRangeWidget(rxCh0, "hardwaregain", "[0 0.5 30]",
									    "Hardware Gain(dB):");
		if(m_widgetGroup)
			m_widgetGroup->add(hwGain1);
		connect(this, &Ad9371::readRequested, hwGain1, &IIOWidget::readAsync);
		rx1Layout->addWidget(hwGain1);

		// #19: RSSI (voltage0 in, read-only)
		IIOWidget *rssi1 = Ad9371WidgetFactory::createReadOnlyWidget(rxCh0, "rssi", "RSSI(dB):");
		if(m_widgetGroup)
			m_widgetGroup->add(rssi1);
		connect(this, &Ad9371::readRequested, rssi1, &IIOWidget::readAsync);
		m_liveWidgets.append(rssi1);
		rx1Layout->addWidget(rssi1);

		// Gain Control label — mirrors gainMode combo directly, no IIO read needed
		QLabel *gainLabel1 = new QLabel("Gain Control: ", rx1Widget);
		Style::setStyle(gainLabel1, style::properties::label::subtle);
		if(gainMode) {
			auto *comboUi = dynamic_cast<ComboAttrUi *>(gainMode->getUiStrategy());
			if(comboUi) {
				connect(comboUi, &ComboAttrUi::displayedNewData, gainLabel1,
					[gainLabel1](const QString &text, const QString &) {
						gainLabel1->setText("Gain Control: " + text);
					});
			}
			QComboBox *combo = gainMode->findChild<QComboBox *>();
			if(combo) {
				connect(combo, &QComboBox::currentTextChanged, gainLabel1,
					[gainLabel1](const QString &text) {
						if(!text.isEmpty())
							gainLabel1->setText("Gain Control: " + text);
					});
			}
		}
		rx1Layout->addWidget(gainLabel1);

		// #11: temp_comp_gain (voltage0 in) [-3, 3, 0.25]
		IIOWidget *tempComp1 = Ad9371WidgetFactory::createRangeWidget(rxCh0, "temp_comp_gain", "[-3 0.25 3]",
									      "Temp Compensation Gain(dB):");
		if(m_widgetGroup)
			m_widgetGroup->add(tempComp1);
		connect(this, &Ad9371::readRequested, tempComp1, &IIOWidget::readAsync);
		rx1Layout->addWidget(tempComp1);

		// #16: quadrature_tracking_en (voltage0 in)
		IIOWidget *quadTrack1 =
			Ad9371WidgetFactory::createCheckboxWidget(rxCh0, "quadrature_tracking_en", "Quadrature");
		if(m_widgetGroup)
			m_widgetGroup->add(quadTrack1);
		connect(this, &Ad9371::readRequested, quadTrack1, &IIOWidget::readAsync);
		rx1Layout->addWidget(quadTrack1);

		channelLayout->addWidget(rx1Widget);
	}

	// RX2 column (conditional on is_2rx_2tx)
	if(m_is2Rx2Tx && rxCh1) {
		QWidget *rx2Widget = new QWidget(content);
		Style::setBackgroundColor(rx2Widget, json::theme::background_primary);
		Style::setStyle(rx2Widget, style::properties::widget::border_interactive);
		QVBoxLayout *rx2Layout = new QVBoxLayout(rx2Widget);
		rx2Widget->setLayout(rx2Layout);

		QLabel *rx2Title = new QLabel("RX2", rx2Widget);
		Style::setStyle(rx2Title, style::properties::label::menuBig);
		rx2Layout->addWidget(rx2Title);

		// #14: hardwaregain (voltage1 in)
		IIOWidget *hwGain2 = Ad9371WidgetFactory::createRangeWidget(rxCh1, "hardwaregain", "[0 0.5 30]",
									    "Hardware Gain(dB):");
		if(m_widgetGroup)
			m_widgetGroup->add(hwGain2);
		connect(this, &Ad9371::readRequested, hwGain2, &IIOWidget::readAsync);
		rx2Layout->addWidget(hwGain2);

		// #20: RSSI (voltage1 in, read-only)
		IIOWidget *rssi2 = Ad9371WidgetFactory::createReadOnlyWidget(rxCh1, "rssi", "RSSI(dB):");
		if(m_widgetGroup)
			m_widgetGroup->add(rssi2);
		connect(this, &Ad9371::readRequested, rssi2, &IIOWidget::readAsync);
		m_liveWidgets.append(rssi2);
		rx2Layout->addWidget(rssi2);

		// Gain Control label — mirrors gainMode combo directly, no IIO read needed
		QLabel *gainLabel2 = new QLabel("Gain Control: ", rx2Widget);
		Style::setStyle(gainLabel2, style::properties::label::subtle);
		if(gainMode) {
			auto *comboUi = dynamic_cast<ComboAttrUi *>(gainMode->getUiStrategy());
			if(comboUi) {
				connect(comboUi, &ComboAttrUi::displayedNewData, gainLabel2,
					[gainLabel2](const QString &text, const QString &) {
						gainLabel2->setText("Gain Control: " + text);
					});
			}
			QComboBox *combo = gainMode->findChild<QComboBox *>();
			if(combo) {
				connect(combo, &QComboBox::currentTextChanged, gainLabel2,
					[gainLabel2](const QString &text) {
						if(!text.isEmpty())
							gainLabel2->setText("Gain Control: " + text);
					});
			}
		}
		rx2Layout->addWidget(gainLabel2);

		// #12: temp_comp_gain (voltage1 in)
		IIOWidget *tempComp2 = Ad9371WidgetFactory::createRangeWidget(rxCh1, "temp_comp_gain", "[-3 0.25 3]",
									      "Temp Compensation Gain(dB):");
		if(m_widgetGroup)
			m_widgetGroup->add(tempComp2);
		connect(this, &Ad9371::readRequested, tempComp2, &IIOWidget::readAsync);
		rx2Layout->addWidget(tempComp2);

		// #17: quadrature_tracking_en (voltage1 in)
		IIOWidget *quadTrack2 =
			Ad9371WidgetFactory::createCheckboxWidget(rxCh1, "quadrature_tracking_en", "Quadrature");
		if(m_widgetGroup)
			m_widgetGroup->add(quadTrack2);
		connect(this, &Ad9371::readRequested, quadTrack2, &IIOWidget::readAsync);
		rx2Layout->addWidget(quadTrack2);

		channelLayout->addWidget(rx2Widget);
	}
	mainLayout->addLayout(channelLayout);

	section->contentLayout()->addWidget(content);
	return section;
}

// --- Transmit Chain ---
QWidget *Ad9371::generateTxChainWidget(QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *content = new QWidget(section);
	Style::setBackgroundColor(content, json::theme::background_primary);
	Style::setStyle(content, style::properties::widget::border_interactive);
	QVBoxLayout *mainLayout = new QVBoxLayout(content);
	content->setLayout(mainLayout);

	iio_channel *txCh0 = iio_device_find_channel(m_dev, "voltage0", true);
	iio_channel *txCh1 = m_is2Rx2Tx ? iio_device_find_channel(m_dev, "voltage1", true) : nullptr;

	// Section-level read-only labels
	QHBoxLayout *sectionControls = new QHBoxLayout();
	if(txCh0) {
		// #42: RF Bandwidth (read-only, MHz)
		IIOWidget *rfBw =
			Ad9371WidgetFactory::createReadOnlyWidget(txCh0, "rf_bandwidth", "RF Bandwidth(MHz)", false);
		if(m_widgetGroup)
			m_widgetGroup->add(rfBw);
		connect(this, &Ad9371::readRequested, rfBw, &IIOWidget::readAsync);
		rfBw->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControls->addWidget(rfBw);

		// #43: Sampling Frequency (read-only, MSPS)
		IIOWidget *sampRate = Ad9371WidgetFactory::createReadOnlyWidget(txCh0, "sampling_frequency",
										"Sampling Rate(MSPS)", false);
		if(m_widgetGroup)
			m_widgetGroup->add(sampRate);
		connect(this, &Ad9371::readRequested, sampRate, &IIOWidget::readAsync);
		sampRate->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControls->addWidget(sampRate);
		m_txSampRateLabel = sampRate;

		// #41: TX LO Frequency (altvoltage1, MHz scale)
		iio_channel *txLo = iio_device_find_channel(m_dev, "altvoltage1", true);
		if(txLo) {
			const char *freqAttr = resolveFreqAttrName(txLo, "TX_LO_frequency");
			IIOWidget *txLoFreq = Ad9371WidgetFactory::createRangeWidget(txLo, freqAttr, "[70 1 6000]",
										     "TX LO Frequency(MHz)");
			if(m_widgetGroup)
				m_widgetGroup->add(txLoFreq);
			connect(this, &Ad9371::readRequested, txLoFreq, &IIOWidget::readAsync);
			txLoFreq->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
			txLoFreq->setUItoDataConversion(
				[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
			sectionControls->addWidget(txLoFreq);
			m_txLoWidget = txLoFreq;
		}
	}
	mainLayout->addLayout(sectionControls);

	// Per-channel layout (side-by-side TX1/TX2)
	QHBoxLayout *channelLayout = new QHBoxLayout();

	// TX1 column
	if(txCh0) {
		QWidget *tx1Widget = new QWidget(content);
		Style::setBackgroundColor(tx1Widget, json::theme::background_primary);
		Style::setStyle(tx1Widget, style::properties::widget::border_interactive);
		QVBoxLayout *tx1Layout = new QVBoxLayout(tx1Widget);
		tx1Widget->setLayout(tx1Layout);

		QLabel *tx1Title = new QLabel("TX1", tx1Widget);
		Style::setStyle(tx1Title, style::properties::label::menuBig);
		tx1Layout->addWidget(tx1Title);

		// #34: hardwaregain (voltage0 out) [0, 41.95, 0.05] inv_scale
		IIOWidget *hwGain1 = Ad9371WidgetFactory::createRangeWidget(txCh0, "hardwaregain", "[0 0.05 41.95]",
									    "Attenuation(dB)");
		if(m_widgetGroup)
			m_widgetGroup->add(hwGain1);
		connect(this, &Ad9371::readRequested, hwGain1, &IIOWidget::readAsync);
		hwGain1->setDataToUIConversion([](QString data) { return QString::number(-data.toDouble(), 'f', 2); });
		hwGain1->setUItoDataConversion([](QString data) { return QString::number(-data.toDouble(), 'f', 2); });
		tx1Layout->addWidget(hwGain1);

		QLabel *trackingLabel1 = new QLabel("Tracking:", tx1Widget);
		tx1Layout->addWidget(trackingLabel1);

		// #37: quadrature_tracking_en (voltage0 out)
		IIOWidget *quadTrack1 =
			Ad9371WidgetFactory::createCheckboxWidget(txCh0, "quadrature_tracking_en", "Quadrature");
		if(m_widgetGroup)
			m_widgetGroup->add(quadTrack1);
		connect(this, &Ad9371::readRequested, quadTrack1, &IIOWidget::readAsync);
		tx1Layout->addWidget(quadTrack1);

		// #38: lo_leakage_tracking_en (voltage0 out)
		IIOWidget *loLeak1 =
			Ad9371WidgetFactory::createCheckboxWidget(txCh0, "lo_leakage_tracking_en", "LO Leakage");
		if(m_widgetGroup)
			m_widgetGroup->add(loLeak1);
		connect(this, &Ad9371::readRequested, loLeak1, &IIOWidget::readAsync);
		tx1Layout->addWidget(loLeak1);

		channelLayout->addWidget(tx1Widget);
	}

	// TX2 column (conditional on is_2rx_2tx)
	if(m_is2Rx2Tx && txCh1) {
		QWidget *tx2Widget = new QWidget(content);
		Style::setBackgroundColor(tx2Widget, json::theme::background_primary);
		Style::setStyle(tx2Widget, style::properties::widget::border_interactive);
		QVBoxLayout *tx2Layout = new QVBoxLayout(tx2Widget);
		tx2Widget->setLayout(tx2Layout);

		QLabel *tx2Title = new QLabel("TX2", tx2Widget);
		Style::setStyle(tx2Title, style::properties::label::menuBig);
		tx2Layout->addWidget(tx2Title);

		// #35: hardwaregain (voltage1 out) inv_scale
		IIOWidget *hwGain2 = Ad9371WidgetFactory::createRangeWidget(txCh1, "hardwaregain", "[0 0.05 41.95]",
									    "Attenuation(dB)");
		if(m_widgetGroup)
			m_widgetGroup->add(hwGain2);
		connect(this, &Ad9371::readRequested, hwGain2, &IIOWidget::readAsync);
		hwGain2->setDataToUIConversion([](QString data) { return QString::number(-data.toDouble(), 'f', 2); });
		hwGain2->setUItoDataConversion([](QString data) { return QString::number(-data.toDouble(), 'f', 2); });
		tx2Layout->addWidget(hwGain2);

		QLabel *trackingLabel2 = new QLabel("Tracking:", tx2Widget);
		tx2Layout->addWidget(trackingLabel2);

		// #39: quadrature_tracking_en (voltage1 out)
		IIOWidget *quadTrack2 =
			Ad9371WidgetFactory::createCheckboxWidget(txCh1, "quadrature_tracking_en", "Quadrature");
		if(m_widgetGroup)
			m_widgetGroup->add(quadTrack2);
		connect(this, &Ad9371::readRequested, quadTrack2, &IIOWidget::readAsync);
		tx2Layout->addWidget(quadTrack2);

		// #40: lo_leakage_tracking_en (voltage1 out)
		IIOWidget *loLeak2 =
			Ad9371WidgetFactory::createCheckboxWidget(txCh1, "lo_leakage_tracking_en", "LO Leakage");
		if(m_widgetGroup)
			m_widgetGroup->add(loLeak2);
		connect(this, &Ad9371::readRequested, loLeak2, &IIOWidget::readAsync);
		tx2Layout->addWidget(loLeak2);

		channelLayout->addWidget(tx2Widget);
	}
	mainLayout->addLayout(channelLayout);

	section->contentLayout()->addWidget(content);
	return section;
}

// --- Observation ---
QWidget *Ad9371::generateObsRxChainWidget(QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *content = new QWidget(section);
	Style::setBackgroundColor(content, json::theme::background_primary);
	Style::setStyle(content, style::properties::widget::border_interactive);
	QVBoxLayout *mainLayout = new QVBoxLayout(content);
	content->setLayout(mainLayout);

	iio_channel *obsCh = iio_device_find_channel(m_dev, "voltage2", false);

	// Section-level controls
	QHBoxLayout *sectionControls = new QHBoxLayout();

	if(obsCh) {
		// #32: RF Bandwidth (read-only, MHz)
		IIOWidget *rfBw =
			Ad9371WidgetFactory::createReadOnlyWidget(obsCh, "rf_bandwidth", "RF Bandwidth(MHz)", false);
		if(m_widgetGroup)
			m_widgetGroup->add(rfBw);
		connect(this, &Ad9371::readRequested, rfBw, &IIOWidget::readAsync);
		rfBw->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		char rfBwBuf[256] = {0};
		int rfBwRet = iio_channel_attr_read(obsCh, "rf_bandwidth", rfBwBuf, sizeof(rfBwBuf));
		if(rfBwRet < 0 || strcmp(rfBwBuf, "ERROR") == 0) {
			rfBw->setEnabled(false);
			rfBw->getUiStrategy()->setInfoMessage("Can't access attribute rf_bandwidth");
		}
		sectionControls->addWidget(rfBw);

		// #33: Sampling Frequency (read-only, MSPS)
		IIOWidget *sampRate = Ad9371WidgetFactory::createReadOnlyWidget(obsCh, "sampling_frequency",
										"Sampling Rate(MSPS)", false);
		if(m_widgetGroup)
			m_widgetGroup->add(sampRate);
		connect(this, &Ad9371::readRequested, sampRate, &IIOWidget::readAsync);
		sampRate->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		sectionControls->addWidget(sampRate);

		// #30: Sniffer LO Frequency (altvoltage2, MHz scale)
		iio_channel *snLo = iio_device_find_channel(m_dev, "altvoltage2", true);
		if(snLo) {
			const char *freqAttr = resolveFreqAttrName(snLo, "RX_SN_LO_frequency");
			IIOWidget *snLoFreq = Ad9371WidgetFactory::createRangeWidget(snLo, freqAttr, "[70 1 6000]",
										     "Sniffer LO Frequency(MHz)");
			if(m_widgetGroup)
				m_widgetGroup->add(snLoFreq);
			connect(this, &Ad9371::readRequested, snLoFreq, &IIOWidget::readAsync);
			snLoFreq->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
			snLoFreq->setUItoDataConversion(
				[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
			sectionControls->addWidget(snLoFreq);
		}

		// #26: RF Port Select
		IIOWidget *portSelect = Ad9371WidgetFactory::createComboWidget(
			obsCh, "rf_port_select", "rf_port_select_available", "Source Select");
		if(m_widgetGroup)
			m_widgetGroup->add(portSelect);
		connect(this, &Ad9371::readRequested, portSelect, &IIOWidget::readAsync);
		sectionControls->addWidget(portSelect);

		// Gain Control SYNC Pulse (debug attribute)
		IIOWidget *obsGcSyncPulse = Ad9371WidgetFactory::createDebugCheckboxWidget(
			m_dev, "adi,obs-agc-conf-agc-enable-sync-pulse-for-gain-counter", "Gain Control SYNC Pulse");
		if(m_widgetGroup)
			m_widgetGroup->add(obsGcSyncPulse);
		connect(this, &Ad9371::readRequested, obsGcSyncPulse, &IIOWidget::readAsync);
		if(iio_device_find_debug_attr(m_dev, "adi,obs-agc-conf-agc-enable-sync-pulse-for-gain-counter") ==
		   nullptr) {
			obsGcSyncPulse->setEnabled(false);
			obsGcSyncPulse->getUiStrategy()->setInfoMessage(
				"Can't access attribute adi,obs-agc-conf-agc-enable-sync-pulse-for-gain-counter");
		}
		sectionControls->addWidget(obsGcSyncPulse);
	}
	mainLayout->addLayout(sectionControls);

	QHBoxLayout *obsChannels = new QHBoxLayout();

	// OBS channel widgets
	if(obsCh) {
		QWidget *obsChWidget = new QWidget(content);
		Style::setBackgroundColor(obsChWidget, json::theme::background_primary);
		Style::setStyle(obsChWidget, style::properties::widget::border_interactive);

		QVBoxLayout *obsChLayout = new QVBoxLayout(obsChWidget);

		// #28: hardwaregain (voltage2 in) [0, 52, 1]
		IIOWidget *hwGain =
			Ad9371WidgetFactory::createRangeWidget(obsCh, "hardwaregain", "[0 1 52]", "Hardware Gain(dB):");
		if(m_widgetGroup)
			m_widgetGroup->add(hwGain);
		connect(this, &Ad9371::readRequested, hwGain, &IIOWidget::readAsync);
		m_liveWidgets.append(hwGain);
		obsChLayout->addWidget(hwGain);

		// #31: RSSI (voltage2 in, read-only)
		IIOWidget *rssi = Ad9371WidgetFactory::createReadOnlyWidget(obsCh, "rssi", "RSSI(dB):");
		if(m_widgetGroup)
			m_widgetGroup->add(rssi);
		connect(this, &Ad9371::readRequested, rssi, &IIOWidget::readAsync);
		m_liveWidgets.append(rssi);
		obsChLayout->addWidget(rssi);

		// Gain Control Mode
		IIOWidget *gainMode = Ad9371WidgetFactory::createComboWidget(
			obsCh, "gain_control_mode", "gain_control_mode_available", "Gain Control Mode");
		if(m_widgetGroup)
			m_widgetGroup->add(gainMode);
		connect(this, &Ad9371::readRequested, gainMode, &IIOWidget::readAsync);
		obsChLayout->addWidget(gainMode);

		// #27: temp_comp_gain (voltage2 in) [-3, 3, 0.25]
		IIOWidget *tempComp = Ad9371WidgetFactory::createRangeWidget(obsCh, "temp_comp_gain", "[-3 0.25 3]",
									     "Temp Compensation Gain(dB):");
		if(m_widgetGroup)
			m_widgetGroup->add(tempComp);
		connect(this, &Ad9371::readRequested, tempComp, &IIOWidget::readAsync);
		obsChLayout->addWidget(tempComp);

		// #29: quadrature_tracking_en (voltage2 in)
		IIOWidget *quadTrack =
			Ad9371WidgetFactory::createCheckboxWidget(obsCh, "quadrature_tracking_en", "Quadrature");
		if(m_widgetGroup)
			m_widgetGroup->add(quadTrack);
		connect(this, &Ad9371::readRequested, quadTrack, &IIOWidget::readAsync);
		obsChLayout->addWidget(quadTrack);

		obsChannels->addWidget(obsChWidget);
	}

	obsChannels->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));
	mainLayout->addLayout(obsChannels);

	section->contentLayout()->addWidget(content);
	return section;
}

// --- FPGA Settings (#100-#101) ---
QWidget *Ad9371::generateFpgaSettingsWidget(QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *content = new QWidget(section);
	Style::setBackgroundColor(content, json::theme::background_primary);
	Style::setStyle(content, style::properties::widget::border_interactive);
	QVBoxLayout *mainLayout = new QVBoxLayout(content);
	content->setLayout(mainLayout);

	// #100: TX Sampling Frequency (dds device, voltage0 out)
	if(m_dds) {
		iio_channel *ddsCh = iio_device_find_channel(m_dds, "voltage0", true);
		if(ddsCh && iio_channel_find_attr(ddsCh, "sampling_frequency_available")) {
			IIOWidget *fpgaTxFreq = Ad9371WidgetFactory::createComboWidget(
				ddsCh, "sampling_frequency", "sampling_frequency_available", "TX Sampling Rate");
			if(m_widgetGroup)
				m_widgetGroup->add(fpgaTxFreq);
			connect(this, &Ad9371::readRequested, fpgaTxFreq, &IIOWidget::readAsync);
			mainLayout->addWidget(fpgaTxFreq);
			m_fpgaTxFreqCombo = fpgaTxFreq;
		}
	}

	// #101: RX Sampling Frequency (cap device, voltage0_i in)
	if(m_cap) {
		iio_channel *capCh = iio_device_find_channel(m_cap, "voltage0_i", false);
		if(capCh && iio_channel_find_attr(capCh, "sampling_frequency_available")) {
			IIOWidget *fpgaRxFreq = Ad9371WidgetFactory::createComboWidget(
				capCh, "sampling_frequency", "sampling_frequency_available", "RX Sampling Rate");
			if(m_widgetGroup)
				m_widgetGroup->add(fpgaRxFreq);
			connect(this, &Ad9371::readRequested, fpgaRxFreq, &IIOWidget::readAsync);
			mainLayout->addWidget(fpgaRxFreq);
			m_fpgaRxFreqCombo = fpgaRxFreq;

			QHBoxLayout *rxLayout = new QHBoxLayout();

			QWidget *rx1Widget = new QWidget(content);
			QHBoxLayout *rx1Layout = new QHBoxLayout();
			rx1Widget->setLayout(rx1Layout);
			Style::setBackgroundColor(rx1Widget, json::theme::background_primary);
			Style::setStyle(rx1Widget, style::properties::widget::border_interactive);
			rxLayout->addWidget(rx1Widget);

			// #23: Phase Rotation (cap device, voltage0_i/q)
			m_phaseSpinbox1 = new gui::MenuSpinbox("Phase Rotation", 0, "°", -180, 180, true, false, false,
							       rx1Widget);
			m_phaseSpinbox1->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
			m_phaseSpinbox1->setScalingEnabled(false);
			m_phaseSpinbox1->enableRangeLimits(false);
			connect(m_phaseSpinbox1, &gui::MenuSpinbox::valueChanged, this,
				[this](double degrees) { writePhase(m_cap, 0, (int)degrees); });
			rx1Layout->addWidget(m_phaseSpinbox1);
			readPhase(m_cap, 0, m_phaseSpinbox1);

			QWidget *rx2Widget = new QWidget(content);
			QHBoxLayout *rx2Layout = new QHBoxLayout();
			rx2Widget->setLayout(rx2Layout);
			Style::setBackgroundColor(rx2Widget, json::theme::background_primary);
			Style::setStyle(rx2Widget, style::properties::widget::border_interactive);
			rxLayout->addWidget(rx2Widget);

			// #24: Phase Rotation (cap device, voltage1_i/q)
			m_phaseSpinbox2 = new gui::MenuSpinbox("Phase Rotation", 0, "°", -180, 180, true, false, false,
							       rx2Widget);
			m_phaseSpinbox2->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
			m_phaseSpinbox2->setScalingEnabled(false);
			m_phaseSpinbox2->enableRangeLimits(false);
			connect(m_phaseSpinbox2, &gui::MenuSpinbox::valueChanged, this,
				[this](double degrees) { writePhase(m_cap, 1, (int)degrees); });
			rx2Layout->addWidget(m_phaseSpinbox2);
			readPhase(m_cap, 1, m_phaseSpinbox2);

			mainLayout->addLayout(rxLayout);
		}
	}

	section->contentLayout()->addWidget(content);
	return section;
}

void Ad9371::writePhase(iio_device *fpgaDev, int channelIndex, int degrees)
{
	double phase = degrees * 2.0 * M_PI / 360.0;

	QString i_ch = QString("voltage%1_i").arg(channelIndex);
	QString q_ch = QString("voltage%1_q").arg(channelIndex);

	iio_channel *i_chn = iio_device_find_channel(fpgaDev, i_ch.toLatin1(), false);
	iio_channel *q_chn = iio_device_find_channel(fpgaDev, q_ch.toLatin1(), false);

	if(i_chn && q_chn) {
		iio_channel_attr_write_double(i_chn, "calibscale", cos(phase));
		iio_channel_attr_write_double(i_chn, "calibphase", -sin(phase));
		iio_channel_attr_write_double(q_chn, "calibscale", cos(phase));
		iio_channel_attr_write_double(q_chn, "calibphase", sin(phase));

		qDebug(CAT_AD9371) << "Phase rotation set to" << degrees << "degrees for channel" << channelIndex;
	} else {
		qWarning(CAT_AD9371) << "Failed to find FPGA channels for phase rotation";
	}
}

void Ad9371::readPhase(iio_device *fpgaDev, int channelIndex, gui::MenuSpinbox *spinBox)
{
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

			double degrees;
			if(round(val[0]) != round(val[1]) && round(val[0]) != round(val[2]) &&
			   round(val[0]) != round(val[3])) {
				qWarning(CAT_AD9371) << "Error calculating phase rotation for channel" << channelIndex;
				degrees = 0.0;
			} else {
				degrees = (val[0] + val[1] + val[2] + val[3]) / 4.0;
			}
			spinBox->setValue(degrees);
		} else {
			qWarning(CAT_AD9371) << "Failed to read phase rotation attributes for channel" << channelIndex;
		}
	}
}

// --- Up/Down Converter ---
void Ad9371::updateLoConversion(IIOWidget *loWidget, bool isRx)
{
	if(!loWidget)
		return;

	if(m_udcEnabled) {
		// When UDC enabled: split frequency between ad9371-phy and ADF4351
		loWidget->setUItoDataConversion([this, isRx](QString data) {
			double targetMHz = data.toDouble();
			const double centerFreq = isRx ? 340.0 : 370.0;
			const double span = 2.0;
			double largePart = floor(targetMHz / span) * span;
			double smallPart = targetMHz - largePart;
			double ad9371LoHz = (centerFreq - smallPart) * 1e6;
			double extPllHz = (centerFreq + largePart) * 1e6;

			// Write to ADF4351 device
			iio_device *udcDev = isRx ? m_udcRx : m_udcTx;
			iio_channel *udcCh = iio_device_find_channel(udcDev, "altvoltage0", true);
			if(udcCh) {
				iio_channel_attr_write_longlong(udcCh, "frequency", (long long)extPllHz);
			}

			// Return ad9371-phy portion for the IIOWidget to write
			return QString::number(ad9371LoHz, 'f', 0);
		});
	} else {
		// Normal mode: simple MHz -> Hz conversion
		loWidget->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	}
}

void Ad9371::onUdcToggled(bool enabled)
{
	m_udcEnabled = enabled;

	// Update LO conversion functions
	updateLoConversion(m_rxLoWidget, true);
	updateLoConversion(m_txLoWidget, false);

	qDebug(CAT_AD9371) << "Up/Down Converter" << (enabled ? "enabled" : "disabled");
}
